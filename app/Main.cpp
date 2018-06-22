#include <QGuiApplication>
#include <QtQuick/QQuickView>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "RenderSurface.h"
#include "ModelerApp.h"

int main(int argc, char **argv) {

    QGuiApplication app(argc, argv);

    qmlRegisterType<Modeler::RenderSurface>("RenderSurface", 1, 0, "RenderSurface");

    QQuickView view;
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:///qml/main.qml"));
    view.show();

    Modeler::ModelerApp modelerApp;
    modelerApp.initialize(&view);
    modelerApp.addLoadedWindow("render_surface", Modeler::ModelerApp::AppWindowType::RenderSurface);

    QQmlApplicationEngine engine;

    //Make my class available in QML.
    view.rootContext()->setContextProperty("_modelerApp",  QVariant::fromValue(&modelerApp));

    return app.exec();

 }
