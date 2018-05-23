#pragma once

#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLFunctions>
#include <QtQuick/qquickwindow.h>

#include "Engine.h"

class RendererGL : public QObject, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    RendererGL() : m_t(0), m_program(nullptr), m_window(nullptr), initialized(false),
                   engineInitialized(false), engine(nullptr) { }
    ~RendererGL();

    void setT(qreal t) { m_t = t; }
    void setViewportSize(const QSize &size) { m_viewportSize = size; }
    void setWindow(QQuickWindow *window) { m_window = window; }

    Core::Engine& getEngine();

public slots:
    void paint();

private:
    QSize m_viewportSize;
    qreal m_t;
    QOpenGLShaderProgram *m_program;
    QQuickWindow *m_window;

    bool initialized;
    bool engineInitialized;
    Core::Engine * engine;

    void init();
    void update();
    void render();
    void testDraw();
};
