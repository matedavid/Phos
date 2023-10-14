#include "asset_watcher.h"

#include "core/uuid.h"

#include "scene/scene.h"
#include "scene/entity.h"
#include "scene/scene_renderer.h"

#include "asset/editor_asset_manager.h"

#include "renderer/backend/renderer.h"
#include "renderer/backend/material.h"
#include "renderer/backend/cubemap.h"

AssetWatcher::AssetWatcher(std::shared_ptr<Phos::Scene> scene,
                           std::shared_ptr<Phos::ISceneRenderer> renderer,
                           std::shared_ptr<Phos::EditorAssetManager> asset_manager)
      : m_scene(std::move(scene)), m_renderer(std::move(renderer)), m_asset_manager(std::move(asset_manager)) {
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
    // @TODO: Doing because modifying material when possible frame in flight (due to multiple frames in flight)
    // Should investigate if I come up with a better approach
    Phos::Renderer::wait_idle();

    if (m_scene->config().environment_config.skybox->id == id) {
        m_scene->config().environment_config.skybox = m_asset_manager->load_by_id_type_force_reload<Phos::Cubemap>(id);
        m_renderer->change_config(m_scene->config());
    }

    if (m_material_to_entities.contains(id)) {
        for (const auto& entity : m_material_to_entities.at(id)) {
            auto& mr = entity.get_component<Phos::MeshRendererComponent>();
            mr.material = m_asset_manager->load_by_id_type_force_reload<Phos::Material>(id);
        }
    }
}
