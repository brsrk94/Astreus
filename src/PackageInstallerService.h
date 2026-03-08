#pragma once

#include <QObject>
#include <QProcess>
#include <QString>

class PackageInstallerService : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString packageFile READ packageFile WRITE setPackageFile NOTIFY packageFileChanged)
    Q_PROPERTY(QString packageType READ packageType NOTIFY packageTypeChanged)
    Q_PROPERTY(QString packageName READ packageName NOTIFY packageNameChanged)
    Q_PROPERTY(QString distro READ distro NOTIFY distroChanged)
    Q_PROPERTY(QString installCommandPreview READ installCommandPreview NOTIFY installCommandPreviewChanged)
    Q_PROPERTY(QString dependencyCommandPreview READ dependencyCommandPreview NOTIFY dependencyCommandPreviewChanged)
    Q_PROPERTY(QString outputLog READ outputLog NOTIFY outputLogChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool validPackage READ validPackage NOTIFY validPackageChanged)
    Q_PROPERTY(bool packageInstalled READ packageInstalled NOTIFY packageInstalledChanged)

public:
    explicit PackageInstallerService(QObject *parent = nullptr);

    QString packageFile() const;
    void setPackageFile(const QString &path);

    QString packageType() const;
    QString packageName() const;
    QString distro() const;

    QString installCommandPreview() const;
    QString dependencyCommandPreview() const;
    QString outputLog() const;

    bool busy() const;
    bool validPackage() const;
    bool packageInstalled() const;

    Q_INVOKABLE void clearLog();
    Q_INVOKABLE void installPackage();
    Q_INVOKABLE void downloadDependencies();
    Q_INVOKABLE void checkDependencies();
    Q_INVOKABLE void uninstallPackage();
    Q_INVOKABLE void cancelRunningTask();

signals:
    void packageFileChanged();
    void packageTypeChanged();
    void packageNameChanged();
    void distroChanged();
    void installCommandPreviewChanged();
    void dependencyCommandPreviewChanged();
    void outputLogChanged();
    void busyChanged();
    void validPackageChanged();
    void packageInstalledChanged();

private:
    enum class PackageKind {
        Deb,
        Rpm,
        Unsupported
    };

    struct CommandSpec {
        QString program;
        QStringList args;
        QString preview;
        bool valid = false;
        QString reason;
    };

    void refreshState();
    void detectDistro();
    PackageKind detectPackageKind(const QString &path) const;
    CommandSpec buildInstallCommand() const;
    CommandSpec buildDependencyCommand() const;
    CommandSpec buildCheckDependenciesCommand() const;
    CommandSpec buildUninstallCommand() const;
    QString resolvePackageNameFromFile() const;
    bool queryInstalledState(const QString &resolvedPackageName) const;

    void runCommand(const CommandSpec &spec, const QString &title);
    void appendLogLine(const QString &line);

    QString m_packageFile;
    QString m_packageType;
    QString m_packageName;
    QString m_distro;

    QString m_installPreview;
    QString m_dependencyPreview;
    QString m_outputLog;

    bool m_busy = false;
    bool m_validPackage = false;
    bool m_packageInstalled = false;

    QProcess m_process;
};
