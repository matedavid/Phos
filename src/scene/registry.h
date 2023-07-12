#pragma once

#include "core.h"

#include <array>
#include <queue>
#include <memory>

#include "scene/component_manager.h"

namespace Phos {

class Registry {
  public:
    Registry();
    ~Registry() = default;

    std::size_t create();
    void destroy(std::size_t entity);

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

    template <typename T>
    [[nodiscard]] std::vector<std::size_t> view() {
        return m_component_manager->get_entities_with_component<T>();
    }

  private:
    std::queue<std::size_t> m_available_ids;
    std::unique_ptr<ComponentManager> m_component_manager;
};

} // namespace Phos
