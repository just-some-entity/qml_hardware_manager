#include <QQmlApplicationEngine>
#include <QGuiApplication>
#include <QtQml>
#include <qqml.h>

#include "brightness.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    qmlRegisterSingletonType<Brightness>("BrightnessControllerP", 1, 0, "BrightnessController",[](QQmlEngine*, QJSEngine*) -> QObject* {
        return new Brightness();
    });

    engine.load(QUrl("/home/entity/projects/qml-brightnessctrl/src/main.qml"));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}