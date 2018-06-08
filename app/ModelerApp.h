#pragma once

#include <vector>
#include <memory>

#include <QGuiApplication>
#include <QtQuick/QQuickView>

#include "ModelerAppWindow.h"
#include "GestureAdapter.h"
#include "PipedEventAdapter.h"
#include "OrbitControls.h"

#include "Core/Engine.h"

namespace Modeler {

    class ModelerApp {
    public:

        const static int MaxWindows = 32;

        enum class AppWindowType {
            None = 0,
            RenderSurface = 1,
        };

        ModelerApp(QQuickView* rootView);
        bool addLoadedWindow(ModelerAppWindow* window, AppWindowType type);
        bool addLoadedWindow(const std::string& windowName, AppWindowType type);

    private:

        void onGesture(GestureAdapter::GestureEvent event);
        void onEngineReady(std::weak_ptr<Core::Engine> engine);

        bool engineReady;
        QQuickView* rootView;
        ModelerAppWindow* liveWindows[MaxWindows];
        OrbitControls* orbitControls;
        std::shared_ptr<Core::Camera> renderCamera;
        std::weak_ptr<Core::Engine> engine;
        PipedEventAdapter<GestureAdapter::GestureEvent> pipedGestureAdapter;
    };
}

