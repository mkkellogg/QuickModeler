#pragma once

#include <QtQuick/QQuickItem>

namespace Modeler {

    // forward declaration
    class ModelerApp;

    class ModelerAppWindow : public QQuickItem
    {
    public:
        ModelerAppWindow();
        virtual ~ModelerAppWindow() = 0;
        virtual bool initialize(ModelerApp* modelerApp);

    protected:
        ModelerApp* modelerApp;

    };
}
