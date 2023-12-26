#pragma once

#include <memory>

namespace Phos {

// Forward declarations
class Scene;
class AssetManagerBase;
class Entity;
class PrefabAsset;

class PrefabLoader {
  public:
    PrefabLoader() = delete;

    [[nodiscard]] static Entity load(const PrefabAsset& prefab,
                                     const std::shared_ptr<Scene>& scene,
                                     const std::shared_ptr<AssetManagerBase>& asset_manager);
};

} // namespace Phos
