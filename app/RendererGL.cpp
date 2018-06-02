#include "RendererGL.h"

#include <QtQuick/qquickwindow.h>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLContext>

namespace Modeler {
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

        for(std::vector<OnInitCallback>::iterator itr = onInits.begin(); itr != onInits.end(); ++itr) {
            OnInitCallback func = *itr;
            func(this);
        }
    }

    void RendererGL::onInit(OnInitCallback func) {
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

    void RendererGL::setViewport(unsigned int hOffset, unsigned int vOffset, unsigned int vpWidth, unsigned int vpHeight) {
        if (engine) {
            engine->setViewport(hOffset, vOffset, vpWidth, vpHeight);
            engineWindowSizeSet = true;
        }
    }

    void RendererGL::setWindow(QQuickWindow *window) {
        m_window = window;
    }
}
