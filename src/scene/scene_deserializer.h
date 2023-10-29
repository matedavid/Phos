#pragma once

#include "core.h"

// Forward declarations
namespace YAML {
class Node;
}

namespace Phos {

// Forward declarations
class Scene;
class Entity;
class AssetManagerBase;

class SceneDeserializer {
  public:
    SceneDeserializer() = delete;

    [[nodiscard]] static std::shared_ptr<Scene> deserialize(const std::string& path,
                                                            const std::shared_ptr<AssetManagerBase>& asset_manager);
};

} // namespace Phos
