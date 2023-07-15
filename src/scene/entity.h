#pragma once

#include "core.h"

#include "core/uuid.h"

#include "scene/registry.h"
#include "scene/components.h"

namespace Phos {

class Entity {
  public:
    Entity() = default;
    Entity(std::size_t id, Registry* registry) : m_id(id), m_registry(registry) {}
    ~Entity() = default;

    Entity(const Entity& entity) = default;
    Entity& operator=(const Entity& entity) = default;

    template <typename T>
    void add_component(T args = {}) {
        m_registry->add_component<T>(m_id, args);
    }

    template <typename T>
    void remove_component() {
        m_registry->remove_component<T>(m_id);
    }

    template <typename T>
    [[nodiscard]] T& get_component() const {
        return m_registry->get_component<T>(m_id);
    }

    [[nodiscard]] UUID uuid() const { return get_component<UUIDComponent>().uuid; }
    [[nodiscard]] std::size_t id() const { return m_id; }

  private:
    std::size_t m_id;
    Registry* m_registry;
};

} // namespace Phos
