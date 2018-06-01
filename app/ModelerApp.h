#pragma once

#include <vector>
#include <memory>

#include <QGuiApplication>
#include <QtQuick/QQuickView>

#include "ModelerAppWindow.h"

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
        QQuickView* rootView;
        ModelerAppWindow* liveWindows[MaxWindows];
    };
}

