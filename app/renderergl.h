#pragma once

#include <vector>
#include <functional>

#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLFunctions>
#include <QtQuick/qquickwindow.h>

#include "Engine.h"

class RendererGL : public QObject, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    RendererGL();
    ~RendererGL();

    void setT(qreal t);
    void setViewportSize(const QSize &size);
    void setWindow(QQuickWindow *window);

    Core::Engine& getEngine();

public slots:
    void paint();
    void onInit(std::function<void()>& func);

private:
    QSize m_viewportSize;
    qreal m_t;
    QOpenGLShaderProgram *m_program;
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
