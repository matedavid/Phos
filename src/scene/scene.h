#pragma once

#include "core.h"

#include "scene/registry.h"
#include "scene/components.h"

namespace Phos {

// Forward declarations
class Camera;
class Entity;

class Scene {
  public:
    explicit Scene(std::string name);
    ~Scene();

    Entity create_entity();
    Entity create_entity(const std::string& name);
    void destroy_entity(Entity entity);

    [[nodiscard]] Entity get_entity_with_uuid(const UUID& uuid);

    template <typename... Components>
    [[nodiscard]] std::vector<Entity> get_entities_with() {
        std::vector<std::size_t> ids = m_registry->view<Components...>();

        std::vector<Entity> entities;
        entities.reserve(ids.size());

        for (const auto& id : ids)
            entities.push_back(*m_id_to_entity[id]);

        return entities;
    }

  private:
    std::string m_name;
    std::unique_ptr<Registry> m_registry;

    // std::shared_ptr<Camera> m_camera;

    std::unordered_map<std::size_t, Entity*> m_id_to_entity;
    std::unordered_map<UUID, Entity*> m_uuid_to_entity;

    friend class Entity;
};

} // namespace Phos
