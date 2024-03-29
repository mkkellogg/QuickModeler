#include <QGuiApplication>
#include <QtQuick/QQuickView>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QGLFormat>

#include "RenderSurface.h"
#include "ModelerApp.h"
#include "TreeModel.h"

int main(int argc, char **argv) {

    QGuiApplication app(argc, argv);

    // Specify an OpenGL 3.3 format using the Core profile.
    // That is, no old-school fixed pipeline functionality
    /*QGLFormat glFormat;
    glFormat.setVersion( 3, 3 );
    glFormat.setProfile( QGLFormat::CoreProfile ); // Requires >=Qt-4.8.0
    glFormat.setSampleBuffers( true );*/


    qmlRegisterType<TreeModel>("TreeModel", 1, 0, "TreeModel" );
    qmlRegisterType<Modeler::RenderSurface>("RenderSurface", 1, 0, "RenderSurface");

    TreeModel model;
    //model.addItem("Test item", "nothing");
   // engine.rootContext()->setContextProperty("theModel", &model);

    QQuickView view;
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:///qml/main.qml"));
    view.show();

    Modeler::ModelerApp modelerApp;
    modelerApp.initialize(&view);
    modelerApp.addLoadedWindow("render_surface", Modeler::ModelerApp::AppWindowType::RenderSurface);

    view.rootContext()->setContextProperty("theModel", QVariant::fromValue(&model));
    view.rootContext()->setContextProperty("_modelerApp",  QVariant::fromValue(&modelerApp));

    return app.exec();

 }
