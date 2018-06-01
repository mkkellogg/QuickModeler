#include <QGuiApplication>
#include <QtQuick/QQuickView>

#include "ModelerApp.h"
#include "RenderSurface.h"

namespace Modeler {
    ModelerApp::ModelerApp(QQuickView* rootView): rootView(rootView) {

    }

    bool ModelerApp::addLoadedWindow(std::shared_ptr<ModelerAppWindow> window) {
        if (std::find(this->appWindows.begin(), this->appWindows.end(), window) != this->appWindows.end()) {
            this->appWindows.push_back(window);
        }

        return true;
    }

    bool ModelerApp::addLoadedWindow(const std::string& windowName) {
        QObject *object = this->rootView->rootObject();
        QObject *rsObj = object->findChild<QObject*>(windowName.c_str());
        if (rsObj) {
            Modeler::RenderSurface* renderSurface = dynamic_cast<Modeler::RenderSurface*>(rsObj);
            if(renderSurface) {
                renderSurface->initialize(this);
            }
            else {
                qDebug() << "Unable to locate instance of RenderSurface!";
                return false;
            }
        }
        return true;
    }
}
