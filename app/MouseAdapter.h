#pragma once

#include <QMouseEvent>
#include <QtQuick/qquickwindow.h>
#include <QtQuick/QQuickItem>

#include "Event.h"
#include "EventQueue.h"

#include "Core/geometry/Vector2.h"

namespace Modeler {

    class MouseAdapter {
    public:

        enum class MouseEventType {
            ButtonDown = 1,
            ButtonUp = 2,
            ButtonClicked = 3,
            Moved = 4,
        };

        class MouseEvent: public Event {
        public:
            MouseEventType type;

        };

        MouseAdapter();

        bool processEvent(QObject* obj, QEvent* event);

    private:
        class MouseButtonStatus {
        public:
            bool pressed;
            Core::Vector2u pressedLocation;
        };

        static const unsigned int MAX_BUTTONS = 16;
        MouseButtonStatus buttonStatuses[MAX_BUTTONS];
        static unsigned int getMouseButtonIndex(const Qt::MouseButton& button);

        EventQueue<MouseEvent> mouseEvents;
    };

}
