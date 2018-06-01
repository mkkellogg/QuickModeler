#pragma once

#include <QtQuick/QQuickItem>

#include "ModelerApp.h"
#include "RendererGL.h"
#include "MouseHandler.h"
#include "Demo.h"
#include "Core/Engine.h"

namespace Modeler  {

    class RenderSurface : public QQuickItem {
        Q_OBJECT
        Q_PROPERTY(qreal t READ t WRITE setT NOTIFY tChanged)

    public:
        RenderSurface();
        ~RenderSurface();

        qreal t() const { return m_t; }
        void setT(qreal t);
        bool initialize(ModelerApp* modelerApp);

    protected:
        bool eventFilter(QObject* obj, QEvent* event);

    signals:
        void tChanged();

    public slots:
        void sync();
        void cleanup();

    private slots:
        void handleWindowChanged(QQuickWindow *win);

    private:
        bool initialized;
        qreal m_t;
        RendererGL* m_renderer;
        Demo* demo;
        MouseHandler mouseHandler;
        ModelerApp* modelerApp;
    };
}
