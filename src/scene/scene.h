#pragma once

#include "core.h"

#include "scene/registry.h"
#include "scene/entity.h"
#include "scene/components.h"

namespace Phos {

class Scene {
  public:
    explicit Scene(std::string name);
    ~Scene() = default;

    Entity create_entity();
    void destroy_entity(Entity entity);

    template <typename... Components>
    std::vector<Entity> get_entities_with() {
        std::vector<std::size_t> ids = m_registry->view<Components...>();

        std::vector<Entity> entities;
        for (const auto id : ids)
            entities.push_back(m_id_to_entity[id]);

        return entities;
    }

  private:
    std::string m_name;
    std::unique_ptr<Registry> m_registry;

    std::unordered_map<std::size_t, Entity> m_id_to_entity;
};

} // namespace Phos
