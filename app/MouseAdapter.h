#pragma once

#include <QMouseEvent>
#include <QtQuick/qquickwindow.h>
#include <QtQuick/QQuickItem>

#include "PipedEventAdapter.h"

#include "Core/geometry/Vector2.h"

namespace Modeler {

    class MouseAdapter {
    public:

        enum class MouseEventType {
            ButtonDown = 0,
            ButtonUp = 1,
            ButtonClicked = 2,
            MouseMoved = 3,
        };

        class MouseEvent {
        public:
            MouseEvent(MouseEventType type): type(type) {}
            MouseEventType getType() {return  type;}
            unsigned int button;
            Core::Vector2u position;
        private:
            MouseEventType type;
        };


        MouseAdapter();

        bool processEvent(QObject* obj, QEvent* event);
        bool setPipedEventAdapter(const PipedEventAdapter<MouseEvent>* adapter);

    private:
        class MouseButtonStatus {
        public:
            bool pressed;
            Core::Vector2u pressedLocation;
        };

        static const unsigned int MAX_BUTTONS = 16;
        MouseButtonStatus buttonStatuses[MAX_BUTTONS];
        static unsigned int getMouseButtonIndex(const Qt::MouseButton& button);

        const PipedEventAdapter<MouseEvent>* pipedEventAdapter;
    };

}
