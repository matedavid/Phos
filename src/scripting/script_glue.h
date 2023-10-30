#pragma once

#include "core.h"

#include <glm/glm.hpp>
#include "scene/components.h"

namespace Phos {

// Forward declarations
class Scene;
class AssetManagerBase;

class ScriptGlue {
  public:
    static void initialize();
    static void shutdown();

    static void set_scene(std::shared_ptr<Scene> scene);
    static void set_asset_manager(std::shared_ptr<AssetManagerBase> asset_manager);

  private:
    // region Entity

    static void Entity_Instantiate(uint64_t prefab_asset_id, uint64_t* id);
    static void Entity_Destroy(uint64_t id);

    // endregion

    // region Input

    static void Input_IsKeyDown(uint32_t key, bool* is_down);
    static void Input_IsMouseButtonDown(uint32_t mouse_button, bool* is_down);

    // endregion

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
