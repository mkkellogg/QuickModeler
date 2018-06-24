#pragma once

#include <vector>
#include <memory>

#include <QGuiApplication>
#include <QtQuick/QQuickView>
#include <QObject>
#include <QString>

#include "ModelerAppWindow.h"
#include "GestureAdapter.h"
#include "PipedEventAdapter.h"
#include "OrbitControls.h"
#include "CoreSync.h"

#include "Core/Engine.h"

namespace Modeler {

    class ModelerApp: public QObject {

        Q_OBJECT

    public:

        const static int MaxWindows = 32;

        enum class AppWindowType {
            None = 0,
            RenderSurface = 1,
        };

        ModelerApp(QObject *parent = 0);
        void initialize(QQuickView* rootView);
        bool addLoadedWindow(ModelerAppWindow* window, AppWindowType type);
        bool addLoadedWindow(const std::string& windowName, AppWindowType type);

    private:

        void onGesture(GestureAdapter::GestureEvent event);
        void onEngineReady(Core::WeakPointer<Core::Engine> engine);

        bool engineReady;
        QQuickView* rootView;
        ModelerAppWindow* liveWindows[MaxWindows];
        std::shared_ptr<OrbitControls> orbitControls;
        Core::WeakPointer<Core::Camera> renderCamera;
        Core::WeakPointer<Core::Engine> engine;
        std::shared_ptr<PipedEventAdapter<GestureAdapter::GestureEvent>> pipedGestureAdapter;
        Core::WeakPointer<Core::Object3D> sceneRoot;
        RenderSurface* renderSurface;
        std::shared_ptr<CoreSync> coreSync;

    public slots:
        void loadModel(const QString& path);
    };
}

