#include "renderergl.h"

#include <QtQuick/qquickwindow.h>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLContext>

RendererGL::RendererGL() : m_t(0), m_program(nullptr), m_window(nullptr), initialized(false),
                                   engineInitialized(false), engineWindowSizeSet(false), engine(nullptr) {

}

RendererGL::~RendererGL()
{
    if (m_program != nullptr) {
        delete m_program;
        m_program = nullptr;
    }

    if (this->engine != nullptr) {
        delete this->engine;
        this->engine = nullptr;
    }
}


void RendererGL::paint()
{
    if (!initialized) {
        init();
        initialized = true;
    }

    update();
    render();

}

Core::Engine& RendererGL::getEngine() {
    return *engine;
}

void RendererGL::init() {
    if (!engineInitialized) {
      engine = new Core::Engine(Core::Engine::GLVersion::Three);
      engine->init();
      engineInitialized = true;
    }

    for(std::vector<std::function<void()>>::iterator itr = onInits.begin(); itr != onInits.end(); ++itr) {
        std::function<void()>& func = *itr;
        func();
    }
}

void RendererGL::onInit(std::function<void()>& func) {
    onInits.push_back(func);
}

void RendererGL::update() {
    engine->update();
}

void RendererGL::render() {
    engine->render();
}

void RendererGL::setT(qreal t) {
    m_t = t;
}

void RendererGL::setViewportSize(const QSize &size) {
    if (m_viewportSize.width() != size.width() || m_viewportSize.height() != size.height() || !engineWindowSizeSet) {
        if (engine) {
            engine->resize(size.width(), size.height());
            engineWindowSizeSet = true;
        }
        m_viewportSize = size;
    }
}

void RendererGL::setWindow(QQuickWindow *window) {
    m_window = window;
}
