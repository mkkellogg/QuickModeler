#include <QGuiApplication>
#include <QtQuick/QQuickView>

#include "RenderSurface.h"
#include "ModelerApp.h"

int main(int argc, char **argv) {

    QGuiApplication app(argc, argv);
    Modeler::ModelerApp modelerApp;

    qmlRegisterType<Modeler::RenderSurface>("RenderSurface", 1, 0, "RenderSurface");

    QQuickView view;
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:///qml/main.qml"));
    view.show();

    QObject *object = view.rootObject();
    QObject *rsObj = object->findChild<QObject*>("render_surface");
    if (rsObj) {
        Modeler::RenderSurface* renderSurface = dynamic_cast<Modeler::RenderSurface*>(rsObj);
        if(renderSurface) {
            renderSurface->initialize(&modelerApp);
        }
        else {
            qDebug() << "Unable to locate instance of RenderSurface!";
            return false;
        }
    }

    return app.exec();

 }
