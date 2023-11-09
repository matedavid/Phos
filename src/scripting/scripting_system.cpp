#include "scripting_system.h"

#include "core/project.h"

#include "asset/asset_manager.h"

#include "scene/scene.h"
#include "scene/entity.h"

#include "scripting/scripting_engine.h"
#include "scripting/script_glue.h"
#include "scripting/class_instance_handle.h"

namespace Phos {

ScriptingSystem::ScriptingSystem(std::shared_ptr<Project> project) : m_project(std::move(project)) {
    const auto project_path = m_project->path();
    m_dll_path = project_path / "bin" / "Debug" / (m_project->name() + ".dll");

    ScriptingEngine::set_dll_path(m_dll_path);

    ScriptGlue::set_scene(m_project->scene());
    ScriptGlue::set_asset_manager(m_project->asset_manager());
}

void ScriptingSystem::on_update(double ts) {
    for (const auto& [entity_id, instance] : m_entity_script_instance) {
        instance->invoke_on_update(ts);
    }
}

void ScriptingSystem::start() {
    m_entity_script_instance.clear();

    // Make copy of current scene
    m_scene_copy = m_project->scene(); // @TODO: The copy :)

    // Create class instances for entities
    for (const auto& entity : m_scene_copy->get_entities_with<ScriptComponent>()) {
        m_entity_script_instance[entity.uuid()] = create_entity_instance(entity);
    }
}

void ScriptingSystem::shutdown() {
    PS_ERROR("[ScriptingSystem::shutdown] Unimplemented");
}

std::shared_ptr<ClassInstanceHandle> ScriptingSystem::create_entity_instance(const Entity& entity) {
    const auto& sc = entity.get_component<ScriptComponent>();

    const auto class_handle = m_project->asset_manager()->load_by_id_type<ClassHandle>(sc.script);
    auto instance = ClassInstanceHandle::instantiate(class_handle);

    instance->invoke_constructor(entity.uuid());

    for (const auto& [name, value] : sc.field_values) {
        if (std::holds_alternative<int32_t>(value)) {
            auto v = std::get<int32_t>(value);
            instance->set_field_value(name, v);
        } else if (std::holds_alternative<float>(value)) {
            auto v = std::get<float>(value);
            instance->set_field_value(name, v);
        } else if (std::holds_alternative<glm::vec3>(value)) {
            auto v = std::get<glm::vec3>(value);
            instance->set_field_value(name, v);
        } else if (std::holds_alternative<std::string>(value)) {
            auto v = std::get<std::string>(value);
            instance->set_field_value(name, v);
        } else if (std::holds_alternative<PrefabRef>(value)) {
            auto v = std::get<PrefabRef>(value);
            instance->set_field_value(name, v.id);
        } else if (std::holds_alternative<EntityRef>(value)) {
            auto v = std::get<EntityRef>(value);
            instance->set_field_value(name, v.id);
        }
    }

    instance->invoke_on_create();

    return instance;
}

} // namespace Phos
