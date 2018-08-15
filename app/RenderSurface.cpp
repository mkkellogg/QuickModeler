#include <QtQuick/qquickwindow.h>
#include "RenderSurface.h"

namespace Modeler {

    RenderSurface::RenderSurface(): initialized(false), m_t(0) {
        connect(this, &QQuickItem::windowChanged, this, &RenderSurface::handleWindowChanged);
    }

    void RenderSurface::setT(qreal t) {
        if (t == m_t) return;
        m_t = t;
        emit tChanged();
        if (window()) window()->update();
    }

    RendererGL& RenderSurface::getRenderer() {
        return renderer;
    }

    bool RenderSurface::eventFilter(QObject* obj, QEvent* event) {

        bool customHandling = mouseAdapter.processEvent(obj, event);
        if (customHandling) return true;

        // standard event processing
        return QObject::eventFilter(obj, event);
    }

    bool RenderSurface::initialize(ModelerApp* modelerApp) {
        if(!ModelerAppWindow::initialize(modelerApp)) return false;
        this->initialized = true;
        return true;
    }

    GestureAdapter* RenderSurface::getGestureAdapter() {
        return &gestureAdapter;
    }

    MouseAdapter* RenderSurface::getMouseAdapter() {
        return &mouseAdapter;
    }

    void RenderSurface::handleWindowChanged(QQuickWindow *win) {
        if (win) {
            connect(win, &QQuickWindow::beforeSynchronizing, this, &RenderSurface::sync, Qt::DirectConnection);
            connect(win, &QQuickWindow::sceneGraphInvalidated, this, &RenderSurface::cleanup, Qt::DirectConnection);
            // If we allow QML to do the clearing, they would clear what we paint
            // and nothing would show.
            win->setClearBeforeRendering(false);
        }
    }

    void RenderSurface::cleanup() {

    }

    void RenderSurface::sync() {
        static QQuickWindow* oldWindow = nullptr;

        if (initialized) {
            QQuickWindow* currentWindow = window();

            if(renderer.isEngineInitialized()) {
                QQuickItem* mouseArea = this->childItems()[0];
                mouseArea->installEventFilter(this);
                gestureAdapter.setMouseAdapter(mouseAdapter);

                QSize windowSize = currentWindow->size() * currentWindow->devicePixelRatio();
                Core::Vector2u engineWidowSize(this->boundingRect().width(), this->boundingRect().height());
                Core::Vector2u engineWindowOffset(this->x(), this->y());
                renderer.setRenderSize(engineWidowSize.x, engineWidowSize.y, engineWindowOffset.x, windowSize.height() - engineWidowSize.y - engineWindowOffset.y, engineWidowSize.x, engineWidowSize.y);
            }

            if (oldWindow != currentWindow) {
                connect(currentWindow, &QQuickWindow::beforeRendering, &renderer, &RendererGL::paint, Qt::DirectConnection);
                oldWindow = currentWindow;
            }

            renderer.setT(m_t);
            renderer.setWindow(currentWindow);
        }
    }

    RenderSurface::~RenderSurface() {

    }
}
