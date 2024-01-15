#pragma once

#include <memory>

// Forward declarations
namespace Phos {

class Entity;
class Scene;
class EditorAssetManager;

} // namespace Phos

class EntityComponentsRenderer {
  public:
    static void display(Phos::Entity& entity,
                        const std::shared_ptr<Phos::Scene>& scene,
                        const std::shared_ptr<Phos::EditorAssetManager>& asset_manager);
};
