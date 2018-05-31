#include <QGuiApplication>

#include <QtQuick/QQuickView>

#include "RenderSurface.h"

int main(int argc, char **argv) {
    QGuiApplication app(argc, argv);

    qmlRegisterType<Modeler::RenderSurface>("RenderSurface", 1, 0, "RenderSurface");

    QQuickView view;
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:///qml/main.qml"));
    view.show();

    return app.exec();
}
