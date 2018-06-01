#pragma once

#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickItem>

#include "ModelerApp.h"
#include "ModelerAppWindow.h"
#include "RendererGL.h"
#include "MouseAdapter.h"
#include "Demo.h"
#include "Core/Engine.h"

namespace Modeler  {

    class RenderSurface : public ModelerAppWindow {
        Q_OBJECT
        Q_PROPERTY(qreal t READ t WRITE setT NOTIFY tChanged)

    public:
        RenderSurface();
        virtual ~RenderSurface() override;

        qreal t() const { return m_t; }
        void setT(qreal t);
        virtual bool initialize(ModelerApp* modelerApp) override;

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
        MouseAdapter mouseAdapter;
    };
}
