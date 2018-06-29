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
#include "Core/material/BasicTexturedMaterial.h"

static const char gridMaterial_vertex[] =
    "#version 100\n"
    "attribute vec4 pos;\n"
    "attribute vec4 color;\n"
    "attribute vec2 uv;\n"
    "uniform mat4 projection;\n"
    "uniform mat4 viewMatrix;\n"
    "uniform mat4 modelMatrix;\n"
    "varying vec4 vColor;\n"
    "varying vec2 vUV;\n"
    "varying vec4 vPos;\n"
    "void main() {\n"
    "    gl_Position = projection * viewMatrix * modelMatrix * pos;\n"
    "    vUV = uv;\n"
    "    vPos = pos;\n"
    "    vColor = color;\n"
    "}\n";

static  const char gridMaterial_fragment[] =
    "#version 100\n"
    "precision mediump float;\n"
    "uniform sampler2D textureA;\n"
    "varying vec4 vColor;\n"
    "varying vec2 vUV;\n"
    "varying vec4 vPos;\n"
    "void main() {\n"
    "    vec4 bounds = vec4(-7.011, -7.011, -6.986, -6.986);\n"
    "    vec4 testPos = vec4(vPos.x, vPos.y, -vPos.x, -vPos.y);\n"
    "    vec4 testResults = step(bounds, testPos);\n"
    "    float alpha = testResults.x * testResults.y * testResults.z * testResults.w;\n"
    "    vec4 textureColor = texture2D(textureA, vUV);\n"
    "    gl_FragColor = vec4(textureColor.rgba) * alpha;\n"
    "}\n";

namespace Modeler {

    class GridMaterial: public Core::BasicTexturedMaterial {
    public:
        GridMaterial(Core::WeakPointer<Core::Graphics> graphics): BasicTexturedMaterial(graphics) {

        }
        Core::Bool build() override {
            const std::string& vertexSrc = gridMaterial_vertex;
            const std::string& fragmentSrc = gridMaterial_fragment;
            Core::Bool ready = this->buildFromSource(vertexSrc, fragmentSrc);
            if (!ready) {
               return false;
            }
            this->bindShaderVarLocations();
            return true;
        }
    };

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

