#pragma once

#include <QMouseEvent>
#include <QtQuick/qquickwindow.h>
#include <QtQuick/QQuickItem>

#include "Core/geometry/Vector2.h"

namespace Modeler {

    class MouseAdapter
    {
        class MouseButtonStatus {
        public:
            bool pressed;
            Core::Vector2u pressedLocation;
        };

        static const unsigned int MAX_BUTTONS = 16;
        MouseButtonStatus buttonStatuses[MAX_BUTTONS];
        static unsigned int getMouseButtonIndex(const Qt::MouseButton& button);

    public:
        MouseAdapter();

        bool processEvent(QObject* obj, QEvent* event);

    };

}
