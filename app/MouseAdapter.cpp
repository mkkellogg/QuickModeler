#include "MouseAdapter.h"

namespace Modeler {

    MouseAdapter::MouseAdapter(): pipedEventAdapter(nullptr) {

    }

    bool MouseAdapter::setPipedEventAdapter(const PipedEventAdapter<MouseEvent>* adapter) {
        if(this->pipedEventAdapter == nullptr) {
            this->pipedEventAdapter = adapter;
            return true;
        }
        else {
            return false;
        }
    }

    bool MouseAdapter::processEvent(QObject* obj, QEvent* event) {

        auto eventType = event->type();
        if (eventType == QEvent::MouseButtonPress ||
            eventType == QEvent::MouseButtonRelease ||
            eventType == QEvent::MouseMove) {

            const QMouseEvent* const mouseEvent = static_cast<const QMouseEvent*>( event );
            unsigned int buttonIndex = getMouseButtonIndex(mouseEvent->button());

            QPoint qMousePos = mouseEvent->pos();
            Core::Vector2i mousePos(qMousePos.x(), qMousePos.y());
            MouseEventType mouseEventType;
            switch(eventType) {
                case QEvent::MouseButtonPress:
                    buttonStatuses[buttonIndex].pressed = true;
                    buttonStatuses[buttonIndex].pressedLocation = mousePos;
                    pressedButtonMask |= 1 << (buttonIndex - 1);
                    mouseEventType = MouseEventType::ButtonDown;
                    break;
                case QEvent::MouseButtonRelease:
                    buttonStatuses[buttonIndex].pressed = false;
                    pressedButtonMask &= ~(1 << (buttonIndex - 1));
                    mouseEventType = MouseEventType::ButtonUp;
                    break;
                case QEvent::MouseMove:
                    mouseEventType = MouseEventType::MouseMoved;
                    break;
                default: break;
            }
            if (this->pipedEventAdapter) {
                MouseEvent event(mouseEventType);
                event.buttons = pressedButtonMask;
                event.position = mousePos;
                this->pipedEventAdapter->accept(event);
            }
            return true;
        }

         return false;
    }

    unsigned int MouseAdapter::getMouseButtonIndex(const Qt::MouseButton& button) {
        if(button == Qt::LeftButton) {return 1;}
        else if(button == Qt::RightButton) {return 2;}
        else if((button == Qt::MiddleButton) || (button == Qt::MidButton)) {return 3;}
        else if(button == Qt::XButton1){return 4;}
        else if(button == Qt::XButton2){return 5;}
        else return 0;
    }

}
