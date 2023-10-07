#include "asset_watcher.h"

#include "core/uuid.h"

#include "scene/scene.h"
#include "scene/entity.h"

#include "asset/editor_asset_manager.h"
#include "asset/asset.h"

#include "renderer/backend/material.h"

AssetWatcher::AssetWatcher(std::shared_ptr<Phos::Scene> scene, std::shared_ptr<Phos::EditorAssetManager> asset_manager)
      : m_scene(std::move(scene)), m_asset_manager(std::move(asset_manager)) {
    for (const auto& entity : m_scene->get_all_entities()) {
        if (entity.has_component<Phos::MeshRendererComponent>()) {
            const auto& mr = entity.get_component<Phos::MeshRendererComponent>();
            if (mr.material != nullptr) {
                m_material_to_entities[mr.material->id].push_back(entity);
            }
        }
    }
}

void AssetWatcher::asset_modified(const Phos::UUID& id) const {
    if (m_material_to_entities.contains(id)) {
        for (const auto& entity : m_material_to_entities.at(id)) {
            auto& mr = entity.get_component<Phos::MeshRendererComponent>();
            mr.material = m_asset_manager->load_by_id_type_force_reload<Phos::Material>(id);
        }
    }
}
