#include "asset_description.h"

#include "managers/shader_manager.h"

#include "asset/asset_loader.h"

#include "renderer/backend/texture.h"
#include "renderer/backend/cubemap.h"
#include "renderer/backend/material.h"
#include "renderer/backend/renderer.h"

namespace Phos {

std::shared_ptr<IAsset> TextureAssetDescription::convert() const {
    return Texture::create(path);
}

std::shared_ptr<IAsset> CubemapAssetDescription::convert() const {
    return Cubemap::create({
        .right = right,
        .left = left,
        .top = top,
        .bottom = bottom,
        .front = front,
        .back = back,
    });
}

std::shared_ptr<IAsset> MaterialAssetDescription::convert() const {
    std::shared_ptr<Shader> shader;
    if (shader_builtin) {
        shader = Renderer::shader_manager()->get_builtin_shader(builtin_shader_name);
    } else {
        PS_FAIL("Only builtin shaders supported at the moment")
    }

    auto material = Material::create(shader, name);

    for (const auto& [property_name, texture] : texture_properties) {
        material->set(property_name, std::dynamic_pointer_cast<Texture>(texture));
    }

    // @TODO:
    return std::shared_ptr<IAsset>();
}

std::shared_ptr<IAsset> MeshAssetDescription::convert() const {
    // @TODO:
    return std::shared_ptr<IAsset>();
}

ModelAssetDescription::~ModelAssetDescription() {
    delete parent_node;
}

std::shared_ptr<IAsset> ModelAssetDescription::convert() const {
    // @TODO:
    return std::shared_ptr<IAsset>();
}

} // namespace Phos