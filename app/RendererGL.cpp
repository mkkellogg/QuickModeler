#include "RendererGL.h"

#include <QtQuick/qquickwindow.h>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLContext>

namespace Modeler {

    RendererGL::RendererGL() : m_t(0), m_window(nullptr), initialized(false), engineInitialized(false), engineWindowSizeSet(false), engine(nullptr) {

    }

    RendererGL::~RendererGL() {

    }

    void RendererGL::paint() {
        if (!initialized) {
            init();
            initialized = true;
        }
        update();
        this->resolveOnUpdates();
        this->resolveOnPreRenders();
        render();
    }

    Core::WeakPointer<Core::Engine> RendererGL::getEngine() {
        return engine;
    }

    void RendererGL::init() {
        if (!engineInitialized) {
          engine = Core::Engine::instance();
          engineInitialized = true;
        }
        resolveOnInits();
    }

    void RendererGL::onInit(LifeCycleEventCallback func) {
        if (engineInitialized) {
            resolveOnInit(func);
        }
        else {
            onInits.push_back(func);
        }
    }

    void RendererGL::onPreRender(LifeCycleEventCallback func) {
        QMutexLocker ml(&this->preRenderMutex);
        onPreRenders.push_back(func);
    }

    void RendererGL::onUpdate(LifeCycleEventCallback func) {
        QMutexLocker ml(&this->updateMutex);
        onUpdates.push_back(func);
    }

    void RendererGL::resolveOnInits() {
        for(std::vector<LifeCycleEventCallback>::iterator itr = onInits.begin(); itr != onInits.end(); ++itr) {
            LifeCycleEventCallback func = *itr;
            resolveOnInit(func);
        }
    }

    void RendererGL::resolveOnInit(LifeCycleEventCallback callback) {
        callback(this);
    }

    void RendererGL::resolveOnUpdates() {
        if (onUpdates.size() > 0) {
            QMutexLocker ml(&this->updateMutex);
            for(std::vector<LifeCycleEventCallback>::iterator itr = onUpdates.begin(); itr != onUpdates.end(); ++itr) {
                LifeCycleEventCallback func = *itr;
                func(this);
            }
            onUpdates.clear();
        }
    }

    void RendererGL::resolveOnPreRenders() {
        if (onPreRenders.size() > 0) {
            QMutexLocker ml(&this->preRenderMutex);
            for(std::vector<LifeCycleEventCallback>::iterator itr = onPreRenders.begin(); itr != onPreRenders.end(); ++itr) {
                LifeCycleEventCallback func = *itr;
                func(this);
            }
            onPreRenders.clear();
        }
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
