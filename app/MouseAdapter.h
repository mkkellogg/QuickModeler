#pragma once

#include <memory>

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
            unsigned int buttons;
            Core::Vector2i position;
        private:
            MouseEventType type;
        };


        MouseAdapter();

        bool processEvent(QObject* obj, QEvent* event);
        bool setPipedEventAdapter(std::weak_ptr<PipedEventAdapter<MouseEvent>> adapter);

    private:
        class MouseButtonStatus {
        public:
            bool pressed;
            Core::Vector2i pressedLocation;
        };

        static const unsigned int MAX_BUTTONS = 16;
        MouseButtonStatus buttonStatuses[MAX_BUTTONS];
        unsigned int pressedButtonMask = 0;
        static unsigned int getMouseButtonIndex(const Qt::MouseButton& button);

        std::weak_ptr<PipedEventAdapter<MouseEvent>> pipedEventAdapter;
    };

}
