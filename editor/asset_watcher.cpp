#include "asset_watcher.h"

#include "core/uuid.h"

#include "scene/scene.h"
#include "scene/scene_renderer.h"

#include "asset/editor_asset_manager.h"

#include "renderer/backend/renderer.h"
#include "renderer/backend/material.h"
#include "renderer/backend/cubemap.h"

AssetWatcher::AssetWatcher(std::shared_ptr<Phos::Scene> scene,
                           std::shared_ptr<Phos::ISceneRenderer> renderer,
                           std::shared_ptr<Phos::EditorAssetManager> asset_manager)
      : m_scene(std::move(scene)), m_renderer(std::move(renderer)), m_asset_manager(std::move(asset_manager)) {
    for (const auto& entry : std::filesystem::recursive_directory_iterator(m_asset_manager->path())) {
        if (entry.path().extension() != ".psa" || entry.is_directory())
            continue;

        const auto id = m_asset_manager->get_asset_id(entry.path());
        const auto type = m_asset_manager->get_asset_type(id);

        if (is_watchable_asset_type(type)) {
            start_watching_asset(entry.path(), type, id);
        }
    }
}

void AssetWatcher::check_asset_modified() {
    for (auto& [path, saved_last_time] : m_watching) {
        const auto& [id, type] = m_path_to_info[path];

        const auto last_time = std::filesystem::last_write_time(path).time_since_epoch().count();
        if (last_time != saved_last_time) {
            PS_INFO("[AssetWatcher] Asset updated: '{}'", path.string());
            saved_last_time = last_time;

            switch (type) {
            case Phos::AssetType::Cubemap:
                update_cubemap(id);
                break;
            case Phos::AssetType::Material:
                update_material(id);
                break;
            case Phos::AssetType::Script:
                update_script(id);
                break;
            case Phos::AssetType::Prefab:
            case Phos::AssetType::Shader:
            case Phos::AssetType::Texture:
            case Phos::AssetType::Mesh:
            case Phos::AssetType::Model:
            case Phos::AssetType::Scene:
                break;
            }
        }
    }
}

void AssetWatcher::asset_created(const std::filesystem::path& path) {
    PS_INFO("[AssetWatcher::asset_created] Added asset: '{}'", path.string());

    const auto id = m_asset_manager->get_asset_id(path);
    const auto type = m_asset_manager->get_asset_type(id);

    if (is_watchable_asset_type(type)) {
        start_watching_asset(path, type, id);
    }
}

void AssetWatcher::asset_renamed(const std::filesystem::path& old_path, const std::filesystem::path& new_path) {
    PS_INFO("[AssetWatcher::asset_renamed] Renamed asset '{}' to '{}'", old_path.string(), new_path.string());

    if (!m_path_to_info.contains(old_path))
        return;

    const auto& [id, type] = m_path_to_info[old_path];

    Phos::Renderer::wait_idle();

    if (type == Phos::AssetType::Cubemap) {
        if (m_scene->config().environment_config.skybox->id == id) {
            m_scene->config().environment_config.skybox =
                m_asset_manager->load_by_id_type_force_reload<Phos::Cubemap>(id);
            m_renderer->change_config(m_scene->config());
        }
    } else if (type == Phos::AssetType::Material) {
        for (const auto& entity : m_scene->get_all_entities()) {
            if (entity.has_component<Phos::MeshRendererComponent>()) {
                auto& mr = entity.get_component<Phos::MeshRendererComponent>();
                if (mr.material != nullptr && mr.material->id == id) {
                    mr.material = m_asset_manager->load_by_id_type_force_reload<Phos::Material>(id);
                }
            }
        }
    } else if (type == Phos::AssetType::Script) {
        PS_ERROR("[AssetWatcher::asset_renamed Script] Unimplemented");
    }

    m_watching.erase(old_path);
    m_path_to_info.erase(new_path);

    start_watching_asset(new_path, type, id);
}

void AssetWatcher::asset_removed(const std::filesystem::path& path) {
    PS_INFO("[AssetWatcher::asset_removed] Removed asset: '{}'", path.string());

    const auto id = m_asset_manager->get_asset_id(path);
    const auto type = m_asset_manager->get_asset_type(id);

    if (!is_watchable_asset_type(type))
        return;

    Phos::Renderer::wait_idle();

    if (type == Phos::AssetType::Cubemap) {
        if (m_scene->config().environment_config.skybox->id == id) {
            m_scene->config().environment_config.skybox = nullptr;
            m_renderer->change_config(m_scene->config());
        }
    } else if (type == Phos::AssetType::Material) {
        for (const auto& entity : m_scene->get_all_entities()) {
            if (entity.has_component<Phos::MeshRendererComponent>()) {
                auto& mr = entity.get_component<Phos::MeshRendererComponent>();
                if (mr.material != nullptr && mr.material->id == id) {
                    mr.material = nullptr;
                }
            }
        }
    } else if (type == Phos::AssetType::Script) {
        PS_ERROR("[AssetWatcher::asset_removed Script] Unimplemented");
    }

    m_watching.erase(path);
    m_path_to_info.erase(path);
}

bool AssetWatcher::is_watchable_asset_type(Phos::AssetType type) {
    return type == Phos::AssetType::Material || type == Phos::AssetType::Script || type == Phos::AssetType::Cubemap;
}

void AssetWatcher::start_watching_asset(const std::filesystem::path& path, Phos::AssetType type, Phos::UUID id) {
    const auto last_time = std::filesystem::last_write_time(path);
    m_watching.insert({path, last_time.time_since_epoch().count()});
    m_path_to_info.insert({path, {id, type}});
}

void AssetWatcher::update_cubemap(const Phos::UUID& asset_id) const {
    if (m_scene->config().environment_config.skybox->id == asset_id) {
        m_scene->config().environment_config.skybox =
            m_asset_manager->load_by_id_type_force_reload<Phos::Cubemap>(asset_id);
        m_renderer->change_config(m_scene->config());
    }
}

void AssetWatcher::update_material(const Phos::UUID& asset_id) const {
    Phos::Renderer::wait_idle();

    for (const auto& entity : m_scene->get_all_entities()) {
        if (entity.has_component<Phos::MeshRendererComponent>()) {
            auto& mr = entity.get_component<Phos::MeshRendererComponent>();
            if (mr.material != nullptr && mr.material->id == asset_id) {
                mr.material = m_asset_manager->load_by_id_type_force_reload<Phos::Material>(asset_id);
            }
        }
    }
}

void AssetWatcher::update_script(const Phos::UUID& asset_id) const {
    (void)asset_id;
    PS_ERROR("[AssetWatcher::update_script] Unimplemented");
}