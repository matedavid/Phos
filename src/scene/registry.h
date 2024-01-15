#pragma once

#include <queue>
#include <memory>

#include "scene/scene_constants.h"
#include "utility/logging.h"
#include "scene/component_manager.h"

namespace Phos {

class Registry {
  public:
    Registry();
    ~Registry() = default;

    std::size_t create();
    void destroy(std::size_t entity);

    template <typename T>
    void add_component(std::size_t entity_id, T component) {
        if (!m_component_manager->contains_component<T>())
            m_component_manager->register_component<T>();

        m_component_manager->add_component(entity_id, component);
    }

    template <typename T>
    void remove_component(std::size_t entity_id) {
        m_component_manager->remove_component<T>(entity_id);
    }

    template <typename T>
    [[nodiscard]] T& get_component(std::size_t entity_id) const {
        return m_component_manager->get_component<T>(entity_id);
    }

    template <typename T>
    [[nodiscard]] bool has_component(std::size_t entity_id) const {
        return m_component_manager->entity_has_component<T>(entity_id);
    }

    template <typename T>
    [[nodiscard]] std::vector<std::size_t> view() const {
        return m_component_manager->get_entities_with_component<T>();
    }

    template <typename T>
    void register_component() {
        if (!m_component_manager->contains_component<T>()) {
            m_component_manager->register_component<T>();
        } else {
            PHOS_LOG_WARNING("Component {} is already registered", typeid(T).name());
        }
    }

    [[nodiscard]] uint32_t number_entities() const {
        return MAX_NUM_ENTITIES - static_cast<uint32_t>(m_available_ids.size());
    }

  private:
    std::queue<std::size_t> m_available_ids;
    std::unique_ptr<ComponentManager> m_component_manager;
};

} // namespace Phos
