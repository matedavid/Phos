#include "asset_watcher.h"

#include "editor_state_manager.h"

#include "utility/logging.h"

#include "core/uuid.h"
#include "core/project.h"

#include "asset/editor_asset_manager.h"
#include "asset/asset_registry.h"

#include "scene/scene.h"
#include "scene/scene_renderer.h"

#include "scripting/class_handle.h"
#include "scripting/scripting_engine.h"
#include "scripting/scripting_system.h"

#include "renderer/mesh.h"
#include "renderer/backend/renderer.h"
#include "renderer/backend/material.h"
#include "renderer/backend/cubemap.h"

AssetWatcher::AssetWatcher(std::shared_ptr<Phos::Scene> scene,
                           std::shared_ptr<Phos::Project> project,
                           std::shared_ptr<Phos::ISceneRenderer> renderer,
                           std::shared_ptr<Phos::ScriptingSystem> scripting)
      : m_scene(std::move(scene)), m_project(std::move(project)), m_renderer(std::move(renderer)),
        m_scripting(std::move(scripting)) {
    m_asset_manager = std::dynamic_pointer_cast<Phos::EditorAssetManager>(m_project->asset_manager());
    PHOS_ASSERT(m_asset_manager != nullptr, "[AssetWatcher] Asset Manager must be of type EditorAssetManager");

    for (const auto& entry : std::filesystem::recursive_directory_iterator(m_asset_manager->path())) {
        if (entry.path().extension() != ".psa" || entry.is_directory())
            continue;

        add_file(entry.path());
    }

    // Also watch dll project path to listen for script updates
    m_dll_path = m_project->scripting_path();
    start_watching_asset(m_dll_path, Phos::AssetType::Script, Phos::UUID(0));

    // Listen to EditorStateManager
    EditorStateManager::subscribe([&](EditorState prev, EditorState new_) {
        if (prev == EditorState::Playing && new_ == EditorState::Editing && m_script_update_pending) {
            update_script();
        }
    });
}

void AssetWatcher::check_asset_modified() {
    for (auto& [path, saved_last_time] : m_watching) {
        const auto& [id, type] = m_path_to_info[path];

        const auto last_time = std::filesystem::last_write_time(path).time_since_epoch().count();
        if (last_time != saved_last_time) {
            PHOS_LOG_INFO("Asset updated: '{}'", path.string());
            saved_last_time = last_time; // saved_last_time is reference, so it's modifying the value in m_watching

            switch (type) {
            case Phos::AssetType::Cubemap:
                update_cubemap(m_asset_manager->load_by_id_type_force_reload<Phos::Cubemap>(id));
                break;
            case Phos::AssetType::Material:
                update_material(m_asset_manager->load_by_id_type_force_reload<Phos::Material>(id));
                break;
            case Phos::AssetType::StaticMesh:
                update_mesh(m_asset_manager->load_by_id_type_force_reload<Phos::StaticMesh>(id));
                break;
            case Phos::AssetType::Script:
                update_script();
                break;
            case Phos::AssetType::Prefab:
            case Phos::AssetType::Shader:
            case Phos::AssetType::Texture:
            case Phos::AssetType::Scene:
                break;
            }
        }
    }
}

void AssetWatcher::asset_created(const std::filesystem::path& path) {
    if (std::filesystem::is_directory(path)) {
        add_directory(path);
    } else {
        add_file(path);
    }
}

void AssetWatcher::asset_renamed(const std::filesystem::path& old_path, const std::filesystem::path& new_path) {
    PHOS_LOG_INFO("Renamed asset '{}' to '{}'", old_path.string(), new_path.string());

    if (!m_path_to_info.contains(old_path))
        return;

    const auto& [id, type] = m_path_to_info[old_path];

    Phos::Renderer::wait_idle();

    if (type == Phos::AssetType::Cubemap) {
        update_cubemap(m_asset_manager->load_by_id_type_force_reload<Phos::Cubemap>(id));
    } else if (type == Phos::AssetType::Material) {
        update_material(m_asset_manager->load_by_id_type_force_reload<Phos::Material>(id));
    } else if (type == Phos::AssetType::StaticMesh) {
        update_mesh(m_asset_manager->load_by_id_type_force_reload<Phos::StaticMesh>(id));
    } else if (type == Phos::AssetType::Script) {
        PHOS_LOG_ERROR("Unimplemented");
        return;
    }

    m_watching.erase(old_path);
    m_path_to_info.erase(new_path);

    start_watching_asset(new_path, type, id);
}

void AssetWatcher::asset_removed(const std::filesystem::path& path) {
    if (std::filesystem::is_directory(path) || path.extension() != ".psa")
        return;

    const auto id = m_asset_manager->get_asset_id(path);
    if (!id) {
        PHOS_LOG_ERROR("Could not remove asset: '{}' because it return invalid id", path.string());
        return;
    }

    const auto type = *m_asset_manager->get_asset_type(*id);

    if (!is_watchable_asset_type(type))
        return;

    PHOS_LOG_INFO("[AssetWatcher::asset_removed] Removed asset: '{}'", path.string());

    Phos::Renderer::wait_idle();

    if (type == Phos::AssetType::Cubemap) {
        const auto& current_skybox = m_scene->config().environment_config.skybox;
        if (current_skybox != nullptr && current_skybox->id == id) {
            m_scene->config().environment_config.skybox = nullptr;
            m_renderer->change_config(m_scene->config());
        }
    } else if (type == Phos::AssetType::Material) {
        for (const auto& entity : m_scene->get_entities_with<Phos::MeshRendererComponent>()) {
            auto& mr = entity.get_component<Phos::MeshRendererComponent>();
            if (mr.material != nullptr && mr.material->id == id) {
                mr.material = nullptr;
            }
        }
    } else if (type == Phos::AssetType::StaticMesh) {
        for (const auto& entity : m_scene->get_entities_with<Phos::MeshRendererComponent>()) {
            auto& mr = entity.get_component<Phos::MeshRendererComponent>();
            if (mr.mesh != nullptr && mr.mesh->id == id) {
                mr.mesh = nullptr;
            }
        }
    } else if (type == Phos::AssetType::Script) {
        PHOS_LOG_ERROR("Unimplemented");
        return;
    }

    m_watching.erase(path);
    m_path_to_info.erase(path);
}

void AssetWatcher::add_file(const std::filesystem::path& path) {
    const auto id = m_asset_manager->get_asset_id(path);
    if (!id) {
        PHOS_LOG_ERROR("Could not add file: '{}' because it return invalid id", path.string());
        return;
    }

    const auto type = *m_asset_manager->get_asset_type(*id);

    if (is_watchable_asset_type(type)) {
        PHOS_LOG_INFO("Added asset: '{}'", path.string());
        start_watching_asset(path, type, *id);
    }
}

void AssetWatcher::add_directory(const std::filesystem::path& path) {
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (!entry.is_directory() && entry.path().extension() == ".psa")
            add_file(entry.path());
    }
}

bool AssetWatcher::is_watchable_asset_type(Phos::AssetType type) {
    return type == Phos::AssetType::Material || type == Phos::AssetType::Cubemap || type == Phos::AssetType::StaticMesh;
}

void AssetWatcher::start_watching_asset(const std::filesystem::path& path, Phos::AssetType type, Phos::UUID id) {
    const auto last_time = std::filesystem::last_write_time(path);
    m_watching.insert({path, last_time.time_since_epoch().count()});
    m_path_to_info.insert({path, {id, type}});
}

void AssetWatcher::update_cubemap(const std::shared_ptr<Phos::Cubemap>& cubemap) const {
    const auto& current_skybox = m_scene->config().environment_config.skybox;
    if (current_skybox != nullptr && current_skybox->id == cubemap->id) {
        m_scene->config().environment_config.skybox = cubemap;
        m_renderer->change_config(m_scene->config());
    }
}

void AssetWatcher::update_material(const std::shared_ptr<Phos::Material>& mat) const {
    Phos::Renderer::wait_idle();

    for (const auto& entity : m_scene->get_entities_with<Phos::MeshRendererComponent>()) {
        auto& mr = entity.get_component<Phos::MeshRendererComponent>();
        if (mr.material != nullptr && mr.material->id == mat->id) {
            mr.material = mat;
        }
    }
}

void AssetWatcher::update_mesh(const std::shared_ptr<Phos::StaticMesh>& mesh) const {
    Phos::Renderer::wait_idle();

    for (const auto& entity : m_scene->get_entities_with<Phos::MeshRendererComponent>()) {
        auto& mr = entity.get_component<Phos::MeshRendererComponent>();
        if (mr.mesh != nullptr && mr.mesh->id == mesh->id) {
            mr.mesh = mesh;
        }
    }
}

void AssetWatcher::update_script() {
    if (EditorStateManager::get_state() == EditorState::Playing) {
        PHOS_LOG_INFO("Scripting engine reloading while playing, changes to scene will apply "
                      "after execution has finished");
        m_script_update_pending = true;
        return;
    }

    m_script_update_pending = false;

    m_asset_manager->remove_asset_type_from_cache(Phos::AssetType::Script);
    m_scripting->shutdown();
    Phos::ScriptingEngine::set_dll_path(m_dll_path);

    // Update all entities with Script Component
    for (const auto& entity : m_scene->get_entities_with<Phos::ScriptComponent>()) {
        auto& sc = entity.get_component<Phos::ScriptComponent>();

        const auto handle = m_asset_manager->load_by_id_type_force_reload<Phos::ClassHandle>(sc.script);
        sc.class_name = handle->class_name();

        const auto field_values = sc.field_values;

        // Remove fields that no longer exist in class
        for (auto it = sc.field_values.begin(); it != sc.field_values.end(); ++it) {
            const auto& name = it->first;

            if (!handle->get_field(name).has_value()) {
                it = sc.field_values.erase(it);
                PHOS_LOG_INFO("Removing not longer existing field: {}", name);
            }
        }

        // Add fields present in class and not present in component
        for (const auto& [name, _, type] : handle->get_all_fields()) {
            if (!sc.field_values.contains(name)) {
                sc.field_values.insert({name, Phos::ClassField::get_default_value(type)});
                PHOS_LOG_INFO("Adding new field: {}", name);
            }
        }
    }
}