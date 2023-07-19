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

class AssetLoader {
  public:
    AssetLoader();
    ~AssetLoader() = default;

    std::shared_ptr<IAsset> load(const std::string& path);

  private:
    std::vector<std::unique_ptr<IAssetParser>> m_parsers;
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
    TextureParser() = default;

    [[nodiscard]] AssetType type() override { return AssetType::Texture; }
    std::shared_ptr<IAsset> parse(const YAML::Node& node, const std::string& path) override;
};

class CubemapParser : public IAssetParser {
  public:
    CubemapParser() = default;

    [[nodiscard]] AssetType type() override { return AssetType::Cubemap; }
    std::shared_ptr<IAsset> parse(const YAML::Node& node, const std::string& path) override;
};

} // namespace Phos
