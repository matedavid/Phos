#pragma once

#include "scene/entity.h"

// Forward declarations
namespace YAML {
class Node;
}

namespace Phos {

// Forward declarations
class Scene;
class AssetManagerBase;

class EntityDeserializer {
  public:
    EntityDeserializer() = delete;

    static Entity deserialize(const YAML::Node& node,
                              const UUID& asset_id,
                              const std::shared_ptr<Scene>& scene,
                              AssetManagerBase* asset_manager);
};

} // namespace Phos
