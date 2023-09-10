#pragma once

#include "core.h"

#include <glm/glm.hpp>

#include "asset/asset.h"
#include "asset/model_asset.h"

// Forward declarations
namespace YAML {
class Node;
}

namespace Phos {

// Forward declarations
class IAssetParser;
class AssetManagerBase;
class Texture;
class ModelAsset;

class AssetLoader {
  public:
    explicit AssetLoader(AssetManagerBase* manager);
    ~AssetLoader() = default;

    [[nodiscard]] UUID get_id(const std::string& path) const;
    [[nodiscard]] std::shared_ptr<IAsset> load(const std::string& path) const;

  private:
    std::vector<std::unique_ptr<IAssetParser>> m_parsers;
    AssetManagerBase* m_manager;
};

//
// Parsers
//

class IAssetParser {
  public:
    virtual ~IAssetParser() = default;

    virtual AssetType type() = 0;
    virtual std::shared_ptr<IAsset> parse(const YAML::Node& node, [[maybe_unused]] const std::string& path) = 0;
};

class TextureParser : public IAssetParser {
  public:
    explicit TextureParser([[maybe_unused]] AssetManagerBase*) {}

    [[nodiscard]] AssetType type() override { return AssetType::Texture; }
    std::shared_ptr<IAsset> parse(const YAML::Node& node, const std::string& path) override;
};

class CubemapParser : public IAssetParser {
  public:
    explicit CubemapParser([[maybe_unused]] AssetManagerBase*) {}

    [[nodiscard]] AssetType type() override { return AssetType::Cubemap; }
    std::shared_ptr<IAsset> parse(const YAML::Node& node, const std::string& path) override;
};

class MaterialParser : public IAssetParser {
  public:
    explicit MaterialParser(AssetManagerBase* manager) : m_manager(manager) {}

    [[nodiscard]] AssetType type() override { return AssetType::Material; }
    std::shared_ptr<IAsset> parse(const YAML::Node& node, const std::string& path) override;

  private:
    AssetManagerBase* m_manager;

    [[nodiscard]] std::shared_ptr<Texture> parse_texture(const YAML::Node& node) const;
    [[nodiscard]] glm::vec3 parse_vec3(const YAML::Node& node) const;
    [[nodiscard]] glm::vec4 parse_vec4(const YAML::Node& node) const;
    [[nodiscard]] float parse_float(const YAML::Node& node) const;
};

class MeshParser : public IAssetParser {
  public:
    explicit MeshParser([[maybe_unused]] AssetManagerBase*) {}

    [[nodiscard]] AssetType type() override { return AssetType::Mesh; }
    std::shared_ptr<IAsset> parse(const YAML::Node& node, const std::string& path) override;
};

class ModelParser : public IAssetParser {
  public:
    explicit ModelParser(AssetManagerBase* manager) : m_manager(manager) {}

    [[nodiscard]] AssetType type() override { return AssetType::Model; }
    std::shared_ptr<IAsset> parse(const YAML::Node& node, const std::string& path) override;

  private:
    AssetManagerBase* m_manager;

    [[nodiscard]] ModelAsset::Node* parse_node_r(const YAML::Node& node) const;
};

} // namespace Phos
