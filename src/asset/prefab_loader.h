#pragma once

#include "core.h"

#include "core/uuid.h"

namespace Phos {

// Forward declarations
class Scene;
class AssetManagerBase;
class Entity;

class PrefabLoader {
  public:
    PrefabLoader() = delete;

    [[nodiscard]] static Entity load(const UUID& prefab_id,
                                     const std::shared_ptr<Scene>& scene,
                                     const std::shared_ptr<AssetManagerBase>& asset_manager);
};

} // namespace Phos
