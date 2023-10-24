#pragma once

#include "core.h"

#include <glm/glm.hpp>
#include "scene/components.h"

namespace Phos {

// Forward declarations
class Scene;

class ScriptGlue {
  public:
    static void initialize();
    static void shutdown();
    static void set_scene(std::shared_ptr<Scene> scene);

  private:
    // region TransformComponent

    static void TransformComponent_GetPosition(uint64_t id, glm::vec3* out);
    static void TransformComponent_SetPosition(uint64_t id, glm::vec3* value);

    static void TransformComponent_GetScale(uint64_t id, glm::vec3* out);
    static void TransformComponent_SetScale(uint64_t id, glm::vec3* value);

    static void TransformComponent_GetRotation(uint64_t id, glm::vec3* out);
    static void TransformComponent_SetRotation(uint64_t id, glm::vec3* value);

    // endregion
};

} // namespace Phos
