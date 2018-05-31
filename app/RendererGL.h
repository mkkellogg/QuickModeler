#pragma once

#include <vector>
#include <functional>

#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLFunctions>
#include <QtQuick/qquickwindow.h>

#include "Core/Engine.h"
#include "Core/geometry/Vector2.h"

namespace Modeler {
    class RendererGL : public QObject, protected QOpenGLFunctions {
        Q_OBJECT
    public:
        RendererGL();
        ~RendererGL();

        void setT(qreal t);
        void setRenderSize(unsigned int width, unsigned int height, bool updateViewport = true);
        void setRenderSize(unsigned int width, unsigned int height, unsigned int hOffset,
                           unsigned int vOffset, unsigned int vpWidth, unsigned int vpHeight);
        void setViewport(unsigned int hOffset, unsigned int vOffset, unsigned int vpWidth, unsigned int vpHeight);
        void setWindow(QQuickWindow *window);

        Core::Engine& getEngine();

    public slots:
        void paint();
        void onInit(std::function<void()>& func);
        bool isEngineInitialized();

    private:
        qreal m_t;
        QQuickWindow *m_window;

        bool initialized;
        bool engineInitialized;
        bool engineWindowSizeSet;
        Core::Engine * engine;

        std::vector<std::function<void()>> onInits;

        void init();
        void update();
        void render();
        void testDraw();
    };
}
