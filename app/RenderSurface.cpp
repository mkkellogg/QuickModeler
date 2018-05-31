#include <QtQuick/qquickwindow.h>
#include "RenderSurface.h"

namespace Modeler {
    RenderSurface::RenderSurface(): m_t(0), m_renderer(nullptr), demo(nullptr) {
        connect(this, &QQuickItem::windowChanged, this, &RenderSurface::handleWindowChanged);
    }

    void RenderSurface::setT(qreal t) {
        if (t == m_t) return;
        m_t = t;
        emit tChanged();
        if (window()) window()->update();
    }

    bool RenderSurface::eventFilter(QObject* obj, QEvent* event) {

        bool customHandling = mouseHandler.handleEvent(obj, event);
        if (customHandling) return true;

        // standard event processing
        return QObject::eventFilter(obj, event);
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
        if (m_renderer) {
            delete m_renderer;
            m_renderer = 0;
        }
    }

    void RenderSurface::sync() {
        static QQuickWindow* oldWindow = nullptr;
        QQuickWindow* currentWindow = window();

        if (!m_renderer) {
            m_renderer = new RendererGL();
            std::function<void()> initer = [this]() {
                if (this->demo == nullptr) {
                    Core::Engine& engine = m_renderer->getEngine();
                    this->demo = new Demo(engine);
                    this->demo->run();
                }
            };
            m_renderer->onInit(initer);

            QQuickItem* mouseArea = this->childItems()[0];
            mouseArea->installEventFilter(this);
        }

        if (oldWindow != currentWindow) {
            connect(currentWindow, &QQuickWindow::beforeRendering, m_renderer, &RendererGL::paint, Qt::DirectConnection);
            oldWindow = currentWindow;
        }

        if(m_renderer->isEngineInitialized()) {
            QSize windowSize = currentWindow->size() * currentWindow->devicePixelRatio();
            Core::Vector2u engineWidowSize(this->boundingRect().width(), this->boundingRect().height());
            Core::Vector2u engineWindowOffset(this->x(), this->y());
            m_renderer->setRenderSize(engineWidowSize.x, engineWidowSize.y, engineWindowOffset.x, windowSize.height() - engineWidowSize.y - engineWindowOffset.y, engineWidowSize.x, engineWidowSize.y);
        }

        m_renderer->setT(m_t);
        m_renderer->setWindow(currentWindow);
    }

    RenderSurface::~RenderSurface() {

    }
}
