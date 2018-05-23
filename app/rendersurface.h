#pragma once

#include <QtQuick/QQuickItem>
#include "renderergl.h"
#include "Engine.h"

class RenderSurface : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(qreal t READ t WRITE setT NOTIFY tChanged)

public:
    RenderSurface();
    ~RenderSurface();

    qreal t() const { return m_t; }
    void setT(qreal t);

signals:
    void tChanged();

public slots:
    void sync();
    void cleanup();

private slots:
    void handleWindowChanged(QQuickWindow *win);

private:
    qreal m_t;
    RendererGL *m_renderer;
    bool engineInitialized;
    Core::Engine * engine;
};
