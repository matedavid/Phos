#pragma once

#include "core.h"

#include "asset/asset.h"

// Forward declarations
namespace YAML {
class Node;
}

namespace Phos {

// Forward declarations
class IAssetParser;
class AssetManager;

class AssetLoader {
  public:
    explicit AssetLoader(AssetManager* manager);
    ~AssetLoader() = default;

    [[nodiscard]] UUID get_id(const std::string& path) const;
    [[nodiscard]] std::shared_ptr<IAsset> load(const std::string& path) const;

  private:
    std::vector<std::unique_ptr<IAssetParser>> m_parsers;
    AssetManager* m_manager;
};

//
// Parsers
//

class IAssetParser {
  public:
    virtual ~IAssetParser() = default;

    virtual AssetType type() = 0;
    virtual std::shared_ptr<IAsset> parse(const YAML::Node& node,
                                          [[maybe_unused]] const std::string& path,
                                          [[maybe_unused]] AssetManager* manager) = 0;
};

class TextureParser : public IAssetParser {
  public:
    TextureParser() = default;

    [[nodiscard]] AssetType type() override { return AssetType::Texture; }
    std::shared_ptr<IAsset> parse(const YAML::Node& node, const std::string& path, AssetManager* manager) override;
};

class CubemapParser : public IAssetParser {
  public:
    CubemapParser() = default;

    [[nodiscard]] AssetType type() override { return AssetType::Cubemap; }
    std::shared_ptr<IAsset> parse(const YAML::Node& node, const std::string& path, AssetManager* manager) override;
};

class MaterialParser : public IAssetParser {
  public:
    MaterialParser() = default;

    [[nodiscard]] AssetType type() override { return AssetType::Material; }
    std::shared_ptr<IAsset> parse(const YAML::Node& node, const std::string& path, AssetManager* manager) override;
};

} // namespace Phos
