#include "PackageInstallerService.h"

#include <QFile>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QTextStream>
#include <QUrl>

namespace {

QString singleQuoted(const QString &value) {
    QString out = value;
    out.replace("'", "'\\''");
    return "'" + out + "'";
}

QString shellCommand(const QStringList &parts) {
    QStringList escaped;
    escaped.reserve(parts.size());
    for (const auto &part : parts) {
        escaped << singleQuoted(part);
    }
    return escaped.join(' ');
}

QString readOsReleaseField(const QString &field) {
    QFile file("/etc/os-release");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    QTextStream stream(&file);
    while (!stream.atEnd()) {
        const QString line = stream.readLine().trimmed();
        if (!line.startsWith(field + '=')) {
            continue;
        }

        QString value = line.mid(field.size() + 1).trimmed();
        if ((value.startsWith('"') && value.endsWith('"')) ||
            (value.startsWith('\'') && value.endsWith('\''))) {
            value = value.mid(1, value.size() - 2);
        }
        return value;
    }
    return {};
}

QString distroPkgManager(const QString &id, const QString &idLike) {
    if (id.contains("opensuse") || idLike.contains("suse")) {
        return "zypper";
    }
    return "dnf";
}

} // namespace

PackageInstallerService::PackageInstallerService(QObject *parent)
    : QObject(parent) {
    detectDistro();

    connect(&m_process, &QProcess::readyReadStandardOutput, this, [this]() {
        appendLogLine(QString::fromUtf8(m_process.readAllStandardOutput()));
    });

    connect(&m_process, &QProcess::readyReadStandardError, this, [this]() {
        appendLogLine(QString::fromUtf8(m_process.readAllStandardError()));
    });

    connect(&m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        appendLogLine("[error] Process error: " + QString::number(static_cast<int>(error)) +
                      " (" + m_process.errorString() + ")\n");
        m_busy = false;
        emit busyChanged();
        refreshState();
    });

    connect(&m_process,
            qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this,
            [this](int code, QProcess::ExitStatus status) {
                if (status == QProcess::CrashExit) {
                    appendLogLine("\n[done] Task crashed.\n");
                } else {
                    appendLogLine("\n[done] Exit code: " + QString::number(code) + "\n");
                }

                m_busy = false;
                emit busyChanged();
                refreshState();
            });

    refreshState();
}

QString PackageInstallerService::packageFile() const {
    return m_packageFile;
}

void PackageInstallerService::setPackageFile(const QString &path) {
    QString normalized = path;
    if (normalized.startsWith("file://")) {
        normalized = QUrl(normalized).toLocalFile();
    }

    if (m_packageFile == normalized) {
        return;
    }

    m_packageFile = normalized;
    emit packageFileChanged();

    refreshState();
}

QString PackageInstallerService::packageType() const {
    return m_packageType;
}

QString PackageInstallerService::packageName() const {
    return m_packageName;
}

QString PackageInstallerService::distro() const {
    return m_distro;
}

QString PackageInstallerService::installCommandPreview() const {
    return m_installPreview;
}

QString PackageInstallerService::dependencyCommandPreview() const {
    return m_dependencyPreview;
}

QString PackageInstallerService::outputLog() const {
    return m_outputLog;
}

bool PackageInstallerService::busy() const {
    return m_busy;
}

bool PackageInstallerService::validPackage() const {
    return m_validPackage;
}

bool PackageInstallerService::packageInstalled() const {
    return m_packageInstalled;
}

void PackageInstallerService::clearLog() {
    m_outputLog.clear();
    emit outputLogChanged();
}

void PackageInstallerService::installPackage() {
    runCommand(buildInstallCommand(), "Install package");
}

void PackageInstallerService::downloadDependencies() {
    runCommand(buildDependencyCommand(), "Download dependencies");
}

void PackageInstallerService::checkDependencies() {
    runCommand(buildCheckDependenciesCommand(), "Check installed dependencies");
}

void PackageInstallerService::uninstallPackage() {
    runCommand(buildUninstallCommand(), "Uninstall package");
}

void PackageInstallerService::cancelRunningTask() {
    if (!m_busy) {
        return;
    }

    appendLogLine("\n[task] Cancel requested...\n");
    m_process.terminate();
}

void PackageInstallerService::refreshState() {
    const auto kind = detectPackageKind(m_packageFile);
    const bool exists = QFileInfo::exists(m_packageFile);

    QString nextType;
    bool nextValid = false;

    switch (kind) {
    case PackageKind::Deb:
        nextType = "Debian package (.deb)";
        nextValid = exists;
        break;
    case PackageKind::Rpm:
        nextType = "RPM package (.rpm/.rhel)";
        nextValid = exists;
        break;
    case PackageKind::Unsupported:
        nextType = "Unsupported (supported: .deb, .rpm, .rhel)";
        nextValid = false;
        break;
    }

    if (m_packageType != nextType) {
        m_packageType = nextType;
        emit packageTypeChanged();
    }

    if (m_validPackage != nextValid) {
        m_validPackage = nextValid;
        emit validPackageChanged();
    }

    const QString nextName = m_validPackage ? resolvePackageNameFromFile() : QString();
    if (m_packageName != nextName) {
        m_packageName = nextName;
        emit packageNameChanged();
    }

    const bool nextInstalled = !m_packageName.isEmpty() ? queryInstalledState(m_packageName) : false;
    if (m_packageInstalled != nextInstalled) {
        m_packageInstalled = nextInstalled;
        emit packageInstalledChanged();
    }

    const auto installSpec = buildInstallCommand();
    const auto depsSpec = buildDependencyCommand();

    if (m_installPreview != installSpec.preview) {
        m_installPreview = installSpec.preview;
        emit installCommandPreviewChanged();
    }

    if (m_dependencyPreview != depsSpec.preview) {
        m_dependencyPreview = depsSpec.preview;
        emit dependencyCommandPreviewChanged();
    }
}

void PackageInstallerService::detectDistro() {
    const QString pretty = readOsReleaseField("PRETTY_NAME");
    const QString id = readOsReleaseField("ID");

    QString text;
    if (!pretty.isEmpty()) {
        text = pretty;
    } else if (!id.isEmpty()) {
        text = id;
    } else {
        text = "Unknown Linux distribution";
    }

    if (m_distro != text) {
        m_distro = text;
        emit distroChanged();
    }
}

PackageInstallerService::PackageKind PackageInstallerService::detectPackageKind(const QString &path) const {
    if (path.isEmpty()) {
        return PackageKind::Unsupported;
    }

    const QString lower = path.toLower();
    if (lower.endsWith(".deb")) {
        return PackageKind::Deb;
    }
    if (lower.endsWith(".rpm") || lower.endsWith(".rhel")) {
        return PackageKind::Rpm;
    }

    return PackageKind::Unsupported;
}

PackageInstallerService::CommandSpec PackageInstallerService::buildInstallCommand() const {
    CommandSpec spec;

    if (!m_validPackage || !QFileInfo::exists(m_packageFile)) {
        spec.preview = "Select a valid local package file first.";
        spec.reason = spec.preview;
        return spec;
    }

    const auto kind = detectPackageKind(m_packageFile);
    const QString idLike = readOsReleaseField("ID_LIKE").toLower();
    const QString id = readOsReleaseField("ID").toLower();

    if (kind == PackageKind::Deb) {
        QStringList cmd = {"apt", "install", "-y", m_packageFile};
        const QString body = shellCommand(cmd);

        spec.program = "pkexec";
        spec.args = {"bash", "-lc", body};
        spec.preview = "pkexec bash -lc " + singleQuoted(body);
        spec.valid = true;
        return spec;
    }

    if (kind == PackageKind::Rpm) {
        const QString manager = distroPkgManager(id, idLike);

        QStringList cmd;
        if (manager == "zypper") {
            cmd = {manager, "--non-interactive", "install", m_packageFile};
        } else {
            cmd = {manager, "install", "-y", m_packageFile};
        }

        const QString body = shellCommand(cmd);
        spec.program = "pkexec";
        spec.args = {"bash", "-lc", body};
        spec.preview = "pkexec bash -lc " + singleQuoted(body);
        spec.valid = true;
        return spec;
    }

    spec.preview = "Unsupported package type.";
    spec.reason = spec.preview;
    return spec;
}

PackageInstallerService::CommandSpec PackageInstallerService::buildDependencyCommand() const {
    CommandSpec spec;

    if (!m_validPackage || !QFileInfo::exists(m_packageFile)) {
        spec.preview = "Select a valid local package file first.";
        spec.reason = spec.preview;
        return spec;
    }

    const auto kind = detectPackageKind(m_packageFile);
    const QString idLike = readOsReleaseField("ID_LIKE").toLower();
    const QString id = readOsReleaseField("ID").toLower();

    if (kind == PackageKind::Deb) {
        QStringList cmd = {"apt", "install", "--download-only", "-y", m_packageFile};
        const QString body = shellCommand(cmd);

        spec.program = "pkexec";
        spec.args = {"bash", "-lc", body};
        spec.preview = "pkexec bash -lc " + singleQuoted(body);
        spec.valid = true;
        return spec;
    }

    if (kind == PackageKind::Rpm) {
        const QString manager = distroPkgManager(id, idLike);

        QStringList cmd;
        if (manager == "zypper") {
            cmd = {manager, "--non-interactive", "install", "--download-only", m_packageFile};
        } else {
            cmd = {manager, "install", "--downloadonly", "-y", m_packageFile};
        }

        const QString body = shellCommand(cmd);
        spec.program = "pkexec";
        spec.args = {"bash", "-lc", body};
        spec.preview = "pkexec bash -lc " + singleQuoted(body);
        spec.valid = true;
        return spec;
    }

    spec.preview = "Unsupported package type.";
    spec.reason = spec.preview;
    return spec;
}

PackageInstallerService::CommandSpec PackageInstallerService::buildCheckDependenciesCommand() const {
    CommandSpec spec;

    if (m_packageName.isEmpty()) {
        spec.preview = "Package metadata not available.";
        spec.reason = spec.preview;
        return spec;
    }

    if (!m_packageInstalled) {
        spec.preview = "Install the package first to check installed dependencies.";
        spec.reason = spec.preview;
        return spec;
    }

    const auto kind = detectPackageKind(m_packageFile);
    if (kind == PackageKind::Deb) {
        QStringList cmd = {"dpkg", "-s", m_packageName};
        const QString body = shellCommand(cmd);
        spec.program = "bash";
        spec.args = {"-lc", body};
        spec.preview = "bash -lc " + singleQuoted(body);
        spec.valid = true;
        return spec;
    }

    if (kind == PackageKind::Rpm) {
        QStringList cmd = {"rpm", "-qR", m_packageName};
        const QString body = shellCommand(cmd);
        spec.program = "bash";
        spec.args = {"-lc", body};
        spec.preview = "bash -lc " + singleQuoted(body);
        spec.valid = true;
        return spec;
    }

    spec.preview = "Unsupported package type.";
    spec.reason = spec.preview;
    return spec;
}

PackageInstallerService::CommandSpec PackageInstallerService::buildUninstallCommand() const {
    CommandSpec spec;

    if (m_packageName.isEmpty()) {
        spec.preview = "Package metadata not available.";
        spec.reason = spec.preview;
        return spec;
    }

    if (!m_packageInstalled) {
        spec.preview = "Package does not appear installed.";
        spec.reason = spec.preview;
        return spec;
    }

    const auto kind = detectPackageKind(m_packageFile);
    const QString idLike = readOsReleaseField("ID_LIKE").toLower();
    const QString id = readOsReleaseField("ID").toLower();

    if (kind == PackageKind::Deb) {
        QStringList cmd = {"apt", "remove", "-y", m_packageName};
        const QString body = shellCommand(cmd);

        spec.program = "pkexec";
        spec.args = {"bash", "-lc", body};
        spec.preview = "pkexec bash -lc " + singleQuoted(body);
        spec.valid = true;
        return spec;
    }

    if (kind == PackageKind::Rpm) {
        const QString manager = distroPkgManager(id, idLike);

        QStringList cmd;
        if (manager == "zypper") {
            cmd = {manager, "--non-interactive", "remove", m_packageName};
        } else {
            cmd = {manager, "remove", "-y", m_packageName};
        }

        const QString body = shellCommand(cmd);
        spec.program = "pkexec";
        spec.args = {"bash", "-lc", body};
        spec.preview = "pkexec bash -lc " + singleQuoted(body);
        spec.valid = true;
        return spec;
    }

    spec.preview = "Unsupported package type.";
    spec.reason = spec.preview;
    return spec;
}

QString PackageInstallerService::resolvePackageNameFromFile() const {
    if (!QFileInfo::exists(m_packageFile)) {
        return {};
    }

    const auto kind = detectPackageKind(m_packageFile);
    QProcess proc;
    proc.setProcessEnvironment(QProcessEnvironment::systemEnvironment());

    if (kind == PackageKind::Deb) {
        proc.start("dpkg-deb", {"-f", m_packageFile, "Package"});
    } else if (kind == PackageKind::Rpm) {
        proc.start("rpm", {"-qp", "--qf", "%{NAME}", m_packageFile});
    } else {
        return {};
    }

    if (!proc.waitForFinished(5000) || proc.exitCode() != 0) {
        return {};
    }

    return QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
}

bool PackageInstallerService::queryInstalledState(const QString &resolvedPackageName) const {
    if (resolvedPackageName.isEmpty()) {
        return false;
    }

    const auto kind = detectPackageKind(m_packageFile);
    QProcess proc;
    proc.setProcessEnvironment(QProcessEnvironment::systemEnvironment());

    if (kind == PackageKind::Deb) {
        proc.start("dpkg", {"-s", resolvedPackageName});
    } else if (kind == PackageKind::Rpm) {
        proc.start("rpm", {"-q", resolvedPackageName});
    } else {
        return false;
    }

    if (!proc.waitForFinished(5000)) {
        return false;
    }

    return proc.exitCode() == 0;
}

void PackageInstallerService::runCommand(const CommandSpec &spec, const QString &title) {
    if (m_busy) {
        appendLogLine("[warn] Another task is already running.\n");
        return;
    }

    if (!spec.valid) {
        appendLogLine("[warn] " + (spec.reason.isEmpty() ? QString("Task not available") : spec.reason) + "\n");
        return;
    }

    appendLogLine("\n[task] " + title + "\n");
    appendLogLine("[cmd] " + spec.preview + "\n\n");

    m_busy = true;
    emit busyChanged();

    m_process.start(spec.program, spec.args);
}

void PackageInstallerService::appendLogLine(const QString &line) {
    m_outputLog += line;
    emit outputLogChanged();
}
