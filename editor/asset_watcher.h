#pragma once

#include "core.h"

#include "core/uuid.h"

namespace Phos {

// Forward declarations
class Scene;
class EditorAssetManager;
class Entity;
class ISceneRenderer;

} // namespace Phos

class AssetWatcher {
  public:
    AssetWatcher(std::shared_ptr<Phos::Scene> scene,
                 std::shared_ptr<Phos::ISceneRenderer> renderer,
                 std::shared_ptr<Phos::EditorAssetManager> asset_manager);
    ~AssetWatcher() = default;

    void asset_modified(const Phos::UUID& id) const;

  private:
    std::shared_ptr<Phos::Scene> m_scene;
    std::shared_ptr<Phos::ISceneRenderer> m_renderer;
    std::shared_ptr<Phos::EditorAssetManager> m_asset_manager;

    // Watching
    std::unordered_map<Phos::UUID, std::vector<Phos::Entity>> m_material_to_entities;
};
