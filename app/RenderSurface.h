#pragma once

#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickItem>

#include "Core/Engine.h"

#include "ModelerApp.h"
#include "ModelerAppWindow.h"
#include "RendererGL.h"
#include "MouseAdapter.h"
#include "GestureAdapter.h"

namespace Modeler  {

    class RenderSurface : public ModelerAppWindow {
        Q_OBJECT
        Q_PROPERTY(qreal t READ t WRITE setT NOTIFY tChanged)

    public:
        RenderSurface();
        virtual ~RenderSurface() override;

        qreal t() const { return m_t; }
        void setT(qreal t);
        RendererGL& getRenderer();

        virtual bool initialize(ModelerApp* modelerApp) override;
        virtual GestureAdapter* getGestureAdapter() override;
        virtual MouseAdapter* getMouseAdapter() override;

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
        RendererGL renderer;
        MouseAdapter mouseAdapter;
        GestureAdapter gestureAdapter;
    };
}
