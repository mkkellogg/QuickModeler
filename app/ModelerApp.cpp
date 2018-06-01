#include <QGuiApplication>
#include <QtQuick/QQuickView>

#include "ModelerApp.h"
#include "RenderSurface.h"

namespace Modeler {
    ModelerApp::ModelerApp(QQuickView* rootView): rootView(rootView), pipedGestureAdapter(std::bind(&ModelerApp::onGesture, this, std::placeholders::_1)) {
        for (unsigned int i = 0; i < MaxWindows; i++) this->liveWindows[i] = nullptr;
    }

    bool ModelerApp::addLoadedWindow(ModelerAppWindow* window, AppWindowType type) {
        if (this->liveWindows[(unsigned int)type] != nullptr) {
            qDebug() << "Window type (" << (int)type << ") already added!";
            return false;
        }

        if (!window->initialize(this)) {
            qDebug() << "Unable to initialize window!";
            return false;
        }
        this->liveWindows[(unsigned int)type] = window;

        if (type == AppWindowType::RenderSurface) {
            GestureAdapter* gestureAdapter = window->getGestureAdapter();
            if (gestureAdapter) {
                gestureAdapter->setPipedEventAdapter(&pipedGestureAdapter);
            }

            RenderSurface* renderSurface = dynamic_cast<RenderSurface*>(window);
            if (renderSurface) {
                RendererGL * renderer = renderSurface->getRenderer();
                std::function<void()> initer = [renderer]() {
                    Core::Engine& engine = renderer->getEngine();
                    Demo* demo = new Demo(engine);
                    demo->run();
                };
                renderer->onInit(initer);
            }

        }

        return true;
    }

    bool ModelerApp::addLoadedWindow(const std::string& windowName, AppWindowType type) {
        QObject *object = this->rootView->rootObject();
        QObject *rsObj = object->findChild<QObject*>(windowName.c_str());
        if (rsObj) {
            ModelerAppWindow* appWindow = dynamic_cast<Modeler::ModelerAppWindow*>(rsObj);
            if(appWindow) {
                return this->addLoadedWindow(appWindow, type);
            }
            else {
                qDebug() << "Unable to locate instance of app window!";
                return false;
            }
        }
        return true;
    }

    void ModelerApp::onGesture(GestureAdapter::GestureEvent event) {
        GestureAdapter::GestureEventType eventType = event.getType();
        switch(eventType) {
            case GestureAdapter::GestureEventType::Drag:
                // printf("App Drag: [%u, %u] -> [%u, %u]\n", event.start.x, event.start.y, event.end.x, event.end.y);
            break;
        }
    }
}
