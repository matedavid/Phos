#include "script_glue.h"

#include <mono/jit/jit.h>

#include "asset/asset_manager.h"
#include "asset/prefab_loader.h"
#include "asset/prefab_asset.h"

#include "scene/scene.h"

#include "input/input.h"

namespace Phos {

#define ADD_INTERNAL_CALL(call) mono_add_internal_call("PhosEngine.InternalCalls::" #call, (void*)call)

static std::shared_ptr<Scene> s_scene;
static std::shared_ptr<AssetManagerBase> s_asset_manager;
static std::function<void(const Entity&)> s_entity_instantiated_callback_func;

void ScriptGlue::initialize() {
    ADD_INTERNAL_CALL(Entity_Instantiate);
    ADD_INTERNAL_CALL(Entity_Destroy);

    ADD_INTERNAL_CALL(Input_IsKeyDown);
    ADD_INTERNAL_CALL(Input_IsMouseButtonDown);

    ADD_INTERNAL_CALL(TransformComponent_GetPosition);
    ADD_INTERNAL_CALL(TransformComponent_SetPosition);
    ADD_INTERNAL_CALL(TransformComponent_GetScale);
    ADD_INTERNAL_CALL(TransformComponent_SetScale);
    ADD_INTERNAL_CALL(TransformComponent_GetRotation);
    ADD_INTERNAL_CALL(TransformComponent_SetRotation);
}

void ScriptGlue::shutdown() {
    s_scene = nullptr;
    s_asset_manager = nullptr;
}

void ScriptGlue::set_scene(std::shared_ptr<Scene> scene) {
    s_scene = std::move(scene);
}

void ScriptGlue::set_asset_manager(std::shared_ptr<AssetManagerBase> asset_manager) {
    s_asset_manager = std::move(asset_manager);
}

void ScriptGlue::set_entity_instantiated_callback(const std::function<void(const Entity&)>& func) {
    s_entity_instantiated_callback_func = func;
}

//
// Internal calls
//

void ScriptGlue::Entity_Instantiate(uint64_t prefab_asset_id, uint64_t* id) {
    const auto prefab_asset = s_asset_manager->load_by_id_type<PrefabAsset>(UUID(prefab_asset_id));
    if (prefab_asset == nullptr) {
        PS_ERROR("[ScriptGlue::Entity_Instantiate] Could not create Prefab with id: {}",
                 static_cast<uint64_t>(prefab_asset_id));
        return;
    }

    const auto entity = PrefabLoader::load(*prefab_asset, s_scene, s_asset_manager);
    s_entity_instantiated_callback_func(entity);
    *id = (uint64_t)entity.uuid();
}

void ScriptGlue::Entity_Destroy(uint64_t id) {
    const auto entity = s_scene->get_entity_with_uuid(Phos::UUID(id));
    s_scene->destroy_entity(entity);
}

void ScriptGlue::Input_IsKeyDown(uint32_t key, bool* is_down) {
    const auto internal_key = static_cast<Key>(key);
    *is_down = Input::is_key_pressed(internal_key);
}

void ScriptGlue::Input_IsMouseButtonDown(uint32_t mouse_button, bool* is_down) {
    const auto internal_mouse = static_cast<MouseButton>(mouse_button);
    *is_down = Input::is_mouse_button_pressed(internal_mouse);
}

void ScriptGlue::TransformComponent_GetPosition(uint64_t id, glm::vec3* out) {
    const auto entity = s_scene->get_entity_with_uuid(Phos::UUID(id));
    *out = entity.get_component<TransformComponent>().position;
}

void ScriptGlue::TransformComponent_SetPosition(uint64_t id, glm::vec3* value) {
    const auto entity = s_scene->get_entity_with_uuid(Phos::UUID(id));
    entity.get_component<TransformComponent>().position = *value;
}

void ScriptGlue::TransformComponent_GetScale(uint64_t id, glm::vec3* out) {
    const auto entity = s_scene->get_entity_with_uuid(Phos::UUID(id));
    *out = entity.get_component<TransformComponent>().scale;
}

void ScriptGlue::TransformComponent_SetScale(uint64_t id, glm::vec3* value) {
    const auto entity = s_scene->get_entity_with_uuid(Phos::UUID(id));
    entity.get_component<TransformComponent>().scale = *value;
}

void ScriptGlue::TransformComponent_GetRotation(uint64_t id, glm::vec3* out) {
    const auto entity = s_scene->get_entity_with_uuid(Phos::UUID(id));
    *out = entity.get_component<TransformComponent>().rotation;
}

void ScriptGlue::TransformComponent_SetRotation(uint64_t id, glm::vec3* value) {
    const auto entity = s_scene->get_entity_with_uuid(Phos::UUID(id));
    entity.get_component<TransformComponent>().rotation = *value;
}

} // namespace Phos
