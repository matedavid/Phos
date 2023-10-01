#pragma once

#include "core.h"
#include <glm/glm.hpp>

#include "core/uuid.h"
#include "asset/asset.h"
#include "asset/model_asset.h"

namespace Phos {

// Forward declarations
class IAsset;

class IAssetDescription {
  public:
    virtual ~IAssetDescription() = default;

    UUID id;
    std::string asset_name;

    [[nodiscard]] virtual AssetType type() const = 0;
    [[nodiscard]] virtual std::shared_ptr<IAsset> convert() const = 0;
};

class TextureAssetDescription : public IAssetDescription {
  public:
    TextureAssetDescription() = default;
    [[nodiscard]] AssetType type() const override { return AssetType::Texture; }

    std::string path;

    [[nodiscard]] std::shared_ptr<IAsset> convert() const override;
};

class CubemapAssetDescription : public IAssetDescription {
  public:
    CubemapAssetDescription() = default;
    [[nodiscard]] AssetType type() const override { return AssetType::Cubemap; }

    std::string left;
    std::string right;
    std::string top;
    std::string bottom;
    std::string front;
    std::string back;

    [[nodiscard]] std::shared_ptr<IAsset> convert() const override;
};

class MaterialAssetDescription : public IAssetDescription {
  public:
    MaterialAssetDescription() = default;
    [[nodiscard]] AssetType type() const override { return AssetType::Material; }

    std::string name;

    bool shader_builtin{};
    std::string builtin_shader_name;

    std::unordered_map<std::string, float> float_properties;
    std::unordered_map<std::string, glm::vec3> vec3_properties;
    std::unordered_map<std::string, glm::vec4> vec4_properties;
    std::unordered_map<std::string, std::shared_ptr<TextureAssetDescription>> texture_properties;

    [[nodiscard]] std::shared_ptr<IAsset> convert() const override;
};

class MeshAssetDescription : public IAssetDescription {
  public:
    MeshAssetDescription() = default;
    [[nodiscard]] AssetType type() const override { return AssetType::Mesh; }

    uint32_t index{};
    std::string model_path;

    [[nodiscard]] std::shared_ptr<IAsset> convert() const override;
};

class ModelAssetDescription : public IAssetDescription {
  public:
    ModelAssetDescription() = default;
    ~ModelAssetDescription() override;

    [[nodiscard]] AssetType type() const override { return AssetType::Model; }

    ModelAsset::Node* parent_node;

    [[nodiscard]] std::shared_ptr<IAsset> convert() const override;
};

} // namespace Phos
