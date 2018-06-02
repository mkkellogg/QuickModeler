#pragma once

#include <vector>
#include <memory>

#include <QGuiApplication>
#include <QtQuick/QQuickView>

#include "ModelerAppWindow.h"
#include "GestureAdapter.h"
#include "PipedEventAdapter.h"

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
        void onEngineReady(Core::Engine& engine);

        QQuickView* rootView;
        ModelerAppWindow* liveWindows[MaxWindows];
        PipedEventAdapter<GestureAdapter::GestureEvent> pipedGestureAdapter;

        std::shared_ptr<Core::Camera> renderCamera;
        Core::Engine* engine;
    };
}

