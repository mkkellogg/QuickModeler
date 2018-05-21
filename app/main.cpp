#include <QGuiApplication>

#include <QtQuick/QQuickView>

#include "renderergl.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<Renderable>("ModelerRenderable", 1, 0, "Renderable");

    QQuickView view;
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:///qml/main.qml"));
    view.show();

    return app.exec();
}
