#include "rendersurface.h"
#include <QtQuick/qquickwindow.h>

RenderSurface::RenderSurface(): m_t(0), m_renderer(nullptr), demo(nullptr)
{
    connect(this, &QQuickItem::windowChanged, this, &RenderSurface::handleWindowChanged);
}

void RenderSurface::setT(qreal t)
{
    if (t == m_t) return;
    m_t = t;
    emit tChanged();
    if (window()) window()->update();
}

void RenderSurface::handleWindowChanged(QQuickWindow *win)
{
    if (win) {
        connect(win, &QQuickWindow::beforeSynchronizing, this, &RenderSurface::sync, Qt::DirectConnection);
        connect(win, &QQuickWindow::sceneGraphInvalidated, this, &RenderSurface::cleanup, Qt::DirectConnection);
        // If we allow QML to do the clearing, they would clear what we paint
        // and nothing would show.
        win->setClearBeforeRendering(false);
    }
}

void RenderSurface::cleanup()
{
    if (m_renderer) {
        delete m_renderer;
        m_renderer = 0;
    }
}

void RenderSurface::sync()
{
    if (!m_renderer) {
        m_renderer = new RendererGL();
        connect(window(), &QQuickWindow::beforeRendering, m_renderer, &RendererGL::paint, Qt::DirectConnection);
    }

    std::function<void()> initer = [this]() {
        if (this->demo == nullptr) {
            Core::Engine& engine = m_renderer->getEngine();
            this->demo = new Core::Demo(engine);

            this->demo->run();
        }
    };

    m_renderer->onInit(initer);
    m_renderer->setViewportSize(window()->size() * window()->devicePixelRatio());
    m_renderer->setT(m_t);
    m_renderer->setWindow(window());
}

RenderSurface::~RenderSurface() {
    if (demo != nullptr) {
        delete demo;
        demo = nullptr;
    }
}
