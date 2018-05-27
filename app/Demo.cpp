#include <memory>

#include "Demo.h"

#include "Core/scene/Scene.h"
#include "Core/common/types.h"
#include "Core/math/Math.h"
#include "Core/math/Matrix4x4.h"
#include "Core/math/Quaternion.h"
#include "Core/render/Camera.h"
#include "Core/material/StandardAttributes.h"
#include "Core/geometry/Mesh.h"
#include "Core/render/RenderableContainer.h"
#include "Core/render/MeshRenderer.h"
#include "Core/material/BasicCubeMaterial.h"
#include "Core/material/BasicMaterial.h"
#include "Core/material/StandardAttributes.h"
#include "Core/image/RawImage.h"
#include "Core/image/CubeTexture.h"

Demo::Demo(Core::Engine& engine): engine(engine) {

}

void Demo::run() {
    engine.onUpdate([this](Core::Engine& engine) {
    static Core::Real rotationAngle = 0.0;
    if (camera) {
        rotationAngle += 0.01;
        if (rotationAngle >= Core::Math::TwoPI) rotationAngle -= Core::Math::TwoPI;

        Core::Quaternion qA;
        qA.fromAngleAxis(rotationAngle, 0, 1, 0);
        Core::Matrix4x4 rotationMatrixA;
        qA.rotationMatrix(rotationMatrixA);

        Core::Matrix4x4 worldMatrix;
        worldMatrix.multiply(rotationMatrixA);
        worldMatrix.translate(12, 0, 0);
        worldMatrix.translate(0, 7, 0);
        camera->getTransform().getLocalMatrix().copy(worldMatrix);
        camera->getTransform().updateWorldMatrix();
        camera->lookAt(Core::Point3r(0, 0, 0));
      }

    });

    std::shared_ptr<Core::Scene> scene = std::make_shared<Core::Scene>();
    engine.setScene(scene);

    std::shared_ptr<Core::Mesh> skyboxMesh = std::make_shared<Core::Mesh>(36, false);
    Core::Real vertexPositions[] = {
        // back
        -1.0, -1.0, -1.0, 1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0,
        -1.0, -1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0, -1.0, 1.0,
        // left
        -1.0, -1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0,
        -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0,
        // right
        1.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 1.0, 1.0, -1.0, 1.0,
        1.0, 1.0, -1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        // top
        -1.0, 1.0, -1.0, 1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, 1.0, 1.0,
        -1.0, 1.0, -1.0, 1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        // bottom
        -1.0, -1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0, -1.0, 1.0, 1.0,
        -1.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 1.0, -1.0, -1.0, 1.0,
        // front
        1.0, 1.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0
    };

    skyboxMesh->enableAttribute(Core::StandardAttributes::Position);
    Core::Bool positionInited = skyboxMesh->initVertexPositions(36);
    ASSERT(positionInited, "Unable to initialize skybox mesh vertex positions.");
    skyboxMesh->getVertexPositions()->store(vertexPositions);

    this->skyboxMaterial = std::make_shared<Core::BasicMaterial>();
    this->skyboxMaterial->build();

    std::shared_ptr<Core::RenderableContainer<Core::Mesh>> skyboxObj = std::make_shared<Core::RenderableContainer<Core::Mesh>>();
    std::shared_ptr<Core::MeshRenderer> skyboxRenderer = std::make_shared<Core::MeshRenderer>(this->skyboxMaterial, skyboxObj);
    skyboxObj->addRenderable(skyboxMesh);
    skyboxObj->setRenderer(skyboxRenderer);
    scene->getRoot()->addObject(skyboxObj);

    camera = std::make_shared<Core::Camera>();
    scene->getRoot()->addObject(camera);

}
