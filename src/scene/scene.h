#pragma once

#include "core.h"

#include "scene/registry.h"
#include "scene/entity.h"
#include "scene/components.h"

namespace Phos {

// Forward declarations
class Camera;

class Scene {
  public:
    explicit Scene(std::string name);
    ~Scene() = default;

    Entity create_entity();
    void destroy_entity(Entity entity);

    // TODO: Camera should be entity with CameraComponent or something similar, but for the moment setting camera
    // directly in the scene
    void set_camera(std::shared_ptr<Camera> camera) { m_camera = std::move(camera); }
    [[nodiscard]] std::shared_ptr<Camera> get_camera() const { return m_camera; }

    template <typename... Components>
    std::vector<Entity> get_entities_with() {
        std::vector<std::size_t> ids = m_registry->view<Components...>();

        std::vector<Entity> entities(ids.size());
        for (uint32_t i = 0; i < ids.size(); ++i)
            entities[i] = m_id_to_entity[ids[i]];

        return entities;
    }

  private:
    std::string m_name;
    std::unique_ptr<Registry> m_registry;

    std::shared_ptr<Camera> m_camera;

    std::unordered_map<std::size_t, Entity> m_id_to_entity;
};

} // namespace Phos
