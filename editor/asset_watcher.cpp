#include "asset_watcher.h"

#include "core/uuid.h"
#include "core/project.h"

#include "asset/editor_asset_manager.h"

#include "scene/scene.h"
#include "scene/scene_renderer.h"

#include "scripting/class_handle.h"
#include "scripting/scripting_engine.h"

#include "renderer/backend/renderer.h"
#include "renderer/backend/material.h"
#include "renderer/backend/cubemap.h"

AssetWatcher::AssetWatcher(std::shared_ptr<Phos::Scene> scene,
                           std::shared_ptr<Phos::Project> project,
                           std::shared_ptr<Phos::ISceneRenderer> renderer)
      : m_scene(std::move(scene)), m_project(std::move(project)), m_renderer(std::move(renderer)) {
    m_asset_manager = std::dynamic_pointer_cast<Phos::EditorAssetManager>(m_project->asset_manager());
    PS_ASSERT(m_asset_manager != nullptr, "[AssetWatcher] Asset Manager must be of type EditorAssetManager")

    for (const auto& entry : std::filesystem::recursive_directory_iterator(m_asset_manager->path())) {
        if (entry.path().extension() != ".psa" || entry.is_directory())
            continue;

        const auto id = m_asset_manager->get_asset_id(entry.path());
        const auto type = m_asset_manager->get_asset_type(id);

        if (is_watchable_asset_type(type)) {
            start_watching_asset(entry.path(), type, id);
        }
    }

    // Also watch dll project path to listen for script updates
    m_dll_path = m_project->path() / "bin" / "Debug" / (m_project->name() + ".dll");
    start_watching_asset(m_dll_path, Phos::AssetType::Script, Phos::UUID(0));
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
                update_script();
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
    return type == Phos::AssetType::Material || type == Phos::AssetType::Cubemap;
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

    for (const auto& entity : m_scene->get_entities_with<Phos::MeshRendererComponent>()) {
        auto& mr = entity.get_component<Phos::MeshRendererComponent>();
        if (mr.material != nullptr && mr.material->id == asset_id) {
            mr.material = m_asset_manager->load_by_id_type_force_reload<Phos::Material>(asset_id);
        }
    }
}

void AssetWatcher::update_script() const {
    Phos::ScriptingEngine::set_dll_path(m_dll_path);

    for (const auto& entity : m_scene->get_entities_with<Phos::ScriptComponent>()) {
        auto& sc = entity.get_component<Phos::ScriptComponent>();

        const auto handle = m_asset_manager->load_by_id_type_force_reload<Phos::ClassHandle>(sc.script);
        sc.class_name = handle->class_name();

        const auto field_values = sc.field_values;

        // Remove fields that do not longer exist in class
        for (const auto& name : field_values | std::views::keys) {
            if (!handle->get_field(name).has_value()) {
                sc.field_values.erase(name);
                PS_INFO("[AssetWatcher::update_script] Removing not longer existing field: {}", name);
            }
        }

        // Add fields present in class and not present in component
        for (const auto& [name, _, type] : handle->get_all_fields()) {
            if (!sc.field_values.contains(name)) {
                sc.field_values.insert({name, Phos::ClassField::get_default_value(type)});
                PS_INFO("[AssetWatcher::update_script] Adding new field: {}", name);
            }
        }
    }
}