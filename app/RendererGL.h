#pragma once

#include <vector>
#include <functional>
#include <memory>

#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLFunctions>
#include <QtQuick/qquickwindow.h>
#include <QMutex>

#include "Core/Engine.h"
#include "Core/geometry/Vector2.h"

namespace Modeler {
    class RendererGL : public QObject, protected QOpenGLFunctions {
        Q_OBJECT

    public:
        typedef std::function<void(RendererGL*)> LifeCycleEventCallback;

        RendererGL();
        ~RendererGL();

        void setT(qreal t);
        void setRenderSize(unsigned int width, unsigned int height, bool updateViewport = true);
        void setRenderSize(unsigned int width, unsigned int height, unsigned int hOffset,
                           unsigned int vOffset, unsigned int vpWidth, unsigned int vpHeight);
        void setViewport(unsigned int hOffset, unsigned int vOffset, unsigned int vpWidth, unsigned int vpHeight);
        void setWindow(QQuickWindow *window);

        Core::WeakPointer<Core::Engine> getEngine();
        void onInit(LifeCycleEventCallback func);
        void onPreRender(LifeCycleEventCallback func);
        bool isEngineInitialized();

    public slots:
        void paint();

    private:
        qreal m_t;
        QQuickWindow *m_window;
        QMutex preRenderMutex;

        bool initialized;
        bool engineInitialized;
        bool engineWindowSizeSet;
        std::shared_ptr<Core::Engine> engine;

        std::vector<LifeCycleEventCallback> onInits;
        std::vector<LifeCycleEventCallback> onPreRenders;

        void init();
        void update();
        void render();
        void testDraw();

        void resolveOnInits();
        void resolveOnInit(LifeCycleEventCallback callback);

        void resolveOnPreRenders();
        void resolveOnPreRender(LifeCycleEventCallback callback);
    };
}
