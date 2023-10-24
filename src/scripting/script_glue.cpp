#include "script_glue.h"

#include <mono/jit/jit.h>

#include "scene/scene.h"
#include "scene/entity.h"

namespace Phos {

#define ADD_INTERNAL_CALL(call) mono_add_internal_call("PhosEngine.InternalCalls::" #call, (void*)call)

static std::shared_ptr<Scene> s_scene;

void ScriptGlue::initialize() {
    ADD_INTERNAL_CALL(Get_Position);
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
void ScriptGlue::Get_Position(uint64_t id, glm::vec3* out) {
    const auto entity = s_scene->get_entity_with_uuid(Phos::UUID(id));
    *out = entity.get_component<TransformComponent>().position;
}

} // namespace Phos
