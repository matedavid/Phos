#include "script_glue.h"

#include <mono/jit/jit.h>

#include "scene/scene.h"
#include "scene/entity.h"

namespace Phos {

#define ADD_INTERNAL_CALL(call) mono_add_internal_call("PhosEngine.InternalCalls::" #call, (void*)call)

static std::shared_ptr<Scene> s_scene;

void ScriptGlue::initialize() {
    ADD_INTERNAL_CALL(TransformComponent_GetPosition);
    ADD_INTERNAL_CALL(TransformComponent_SetPosition);
    ADD_INTERNAL_CALL(TransformComponent_GetScale);
    ADD_INTERNAL_CALL(TransformComponent_SetScale);
    ADD_INTERNAL_CALL(TransformComponent_GetRotation);
    ADD_INTERNAL_CALL(TransformComponent_SetRotation);
}

void ScriptGlue::shutdown() {
    s_scene = nullptr;
}

void ScriptGlue::set_scene(std::shared_ptr<Scene> scene) {
    s_scene = std::move(scene);
}

//
// Internal calls
//

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
