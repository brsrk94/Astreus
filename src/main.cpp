#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "PackageInstallerService.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    PackageInstallerService service;

    engine.rootContext()->setContextProperty("installerService", &service);
    engine.loadFromModule("Astreus", "Main");

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
