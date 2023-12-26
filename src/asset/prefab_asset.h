#pragma once

#include "asset/asset.h"

namespace Phos {

class PrefabAsset : public IAsset {
  public:
    explicit PrefabAsset(std::string str) { components_string_representation = std::move(str); }
    ~PrefabAsset() override = default;

    [[nodiscard]] AssetType asset_type() override { return AssetType::Prefab; }

    std::string components_string_representation;
};

} // namespace Phos