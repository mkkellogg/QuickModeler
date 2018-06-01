#pragma once

#include <vector>
#include <memory>

#include <QGuiApplication>
#include <QtQuick/QQuickView>

#include "ModelerAppWindow.h"

namespace Modeler {

    class ModelerApp {
    public:
        ModelerApp(QQuickView* rootView);
        bool addLoadedWindow(std::shared_ptr<ModelerAppWindow> window);
        bool addLoadedWindow(const std::string& windowName);

    private:
        QQuickView* rootView;
        std::vector<std::shared_ptr<ModelerAppWindow>> appWindows;
    };
}

