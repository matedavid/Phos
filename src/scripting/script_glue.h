#pragma once

#include <memory>
#include <functional>

#include <glm/glm.hpp>
#include "scene/components.h"

namespace Phos {

// Forward declarations
class Scene;
class AssetManagerBase;
class Entity;

class ScriptGlue {
  public:
    static void initialize();
    static void shutdown();

    static void set_scene(std::shared_ptr<Scene> scene);
    static void set_asset_manager(std::shared_ptr<AssetManagerBase> asset_manager);

    static void set_entity_instantiated_callback(const std::function<void(const Entity&)>& func);
    static void set_entity_destroyed_callback(const std::function<void(const Entity&)>& func);

  private:
    // region Logging

    static void Logging_Info(MonoString* content);
    static void Logging_Warning(MonoString* content);
    static void Logging_Error(MonoString* content);

    // endregion

    // region Entity

    static void Entity_Instantiate(uint64_t prefab_asset_id, uint64_t* id);
    static void Entity_Destroy(uint64_t id);

    // endregion

    // region Input

    static void Input_IsKeyDown(uint32_t key, bool* is_down);
    static void Input_IsMouseButtonDown(uint32_t mouse_button, bool* is_down);
    static void Input_HorizontalAxisChange(float* out);
    static void Input_VerticalAxisChange(float* out);

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
