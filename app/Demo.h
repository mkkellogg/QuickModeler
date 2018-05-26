#pragma once

#include "Core/Engine.h"
#include "Core/image/RawImage.h"
#include "Core/image/CubeTexture.h"
#include "Core/material/BasicMaterial.h"

class Demo {
public:
    Demo(Core::Engine& engine);
    void run();

private:
    Core::Engine& engine;

    std::vector<std::shared_ptr<Core::RawImage>> skyboxImages;
    std::shared_ptr<Core::CubeTexture> skyboxTexture;
    std::shared_ptr<Core::BasicMaterial> skyboxMaterial;
    std::shared_ptr<Core::ImageLoader> imageLoader;
    std::shared_ptr<Core::AssetLoader> assetLoader;

};
