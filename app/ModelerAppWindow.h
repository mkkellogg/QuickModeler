#pragma once

#include <QtQuick/QQuickItem>

#include "GestureAdapter.h"

namespace Modeler {

    // forward declaration
    class ModelerApp;

    class ModelerAppWindow : public QQuickItem
    {
    public:
        ModelerAppWindow();
        virtual ~ModelerAppWindow() = 0;
        virtual bool initialize(ModelerApp* modelerApp);
        virtual GestureAdapter* getGestureAdapter() = 0;
    protected:
        ModelerApp* modelerApp;

    };
}
