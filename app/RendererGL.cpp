#include "RendererGL.h"

#include <QtQuick/qquickwindow.h>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLContext>

RendererGL::RendererGL() : m_t(0), m_window(nullptr), initialized(false),
                                   engineInitialized(false), engineWindowSizeSet(false), engine(nullptr) {

}

RendererGL::~RendererGL() {
    if (this->engine != nullptr) {
        delete this->engine;
        this->engine = nullptr;
    }
}


void RendererGL::paint() {
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

bool RendererGL::isEngineInitialized() {
    return this->engineInitialized;
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

void RendererGL::setRenderSize(unsigned int width, unsigned int height, bool updateViewport) {
    if (engine) {
        engine->setRenderSize(width, height, updateViewport);
        engineWindowSizeSet = true;
    }
}

void RendererGL::setRenderSize(unsigned int width, unsigned int height, unsigned int hOffset,
                               unsigned int vOffset, unsigned int vpWidth, unsigned int vpHeight) {
    if (engine) {
        engine->setRenderSize(width, height, hOffset, vOffset, vpWidth, vpHeight);
        engineWindowSizeSet = true;
    }

}

void RendererGL::setWindow(QQuickWindow *window) {
    m_window = window;
}
