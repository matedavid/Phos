#pragma once

#include "core.h"

#include <array>
#include <queue>
#include <memory>

#include "ecs/component_manager.h"

namespace Phos {

// Forward declarations
class Entity;

class Registry {
  public:
    Registry();
    ~Registry() = default;

    std::shared_ptr<Entity> create_entity();
    void destroy_entity(const std::shared_ptr<Entity>& entity);

  private:
    std::queue<std::size_t> m_available_ids;
    std::array<std::shared_ptr<Entity>, MAX_NUM_ENTITIES> m_entities;

    std::unique_ptr<ComponentManager> m_component_manager;

    template <typename T, class... Types>
    void add_component(std::size_t entity_id, Types... args) {
        if (!m_component_manager->contains_component<T>())
            m_component_manager->register_component<T>();

        const auto component = T{args...};
        m_component_manager->add_component(entity_id, component);
    }

    template <typename T>
    void remove_component(std::size_t entity_id) {
        m_component_manager->remove_component<T>(entity_id);
    }

    template <typename T>
    T& get_component(std::size_t entity_id) {
        return m_component_manager->get_component<T>(entity_id);
    }

    friend class Entity;
};

} // namespace Phos
