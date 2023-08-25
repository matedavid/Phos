#pragma once

#include "core.h"

#include <unordered_map>

#include "scene/component_array.h"

namespace Phos {

class ComponentManager {
  public:
    template <typename T>
    void register_component() {
        const std::string type_name = typeid(T).name();

        if (m_component_arrays.contains(type_name)) {
            PS_ERROR("Component {} is already registered", type_name);
            return;
        }

        m_component_arrays.insert({type_name, std::make_shared<ComponentArray<T>>()});
    }

    template <typename T>
    [[nodiscard]] bool contains_component() {
        const std::string type_name = typeid(T).name();
        return m_component_arrays.contains(type_name);
    }

    template <typename T>
    [[nodiscard]] T& get_component(std::size_t entity_id) const {
        return get_component_array<T>()->get_data(entity_id);
    }

    template <typename T>
    void add_component(std::size_t entity_id, T component) {
        get_component_array<T>()->insert_data(entity_id, component);
    }

    template <typename T>
    void remove_component(std::size_t entity_id) {
        get_component_array<T>()->remove_data(entity_id);
    }

    template <typename T>
    [[nodiscard]] bool entity_has_component(std::size_t entity_id) const {
        return get_component_array<T>()->entity_has_component(entity_id);
    }

    template <typename T>
    [[nodiscard]] std::vector<std::size_t> get_entities_with_component() const {
        return get_component_array<T>()->get_entities();
    }

    [[nodiscard]] std::vector<std::string> get_component_names(std::size_t entity_id) {
        std::vector<std::string> names;
        for (const auto& [name, _] : m_component_arrays) {
            names.push_back(name);
        }

        return names;
    }

    void entity_destroyed(std::size_t entity_id) {
        for (const auto& [_, array] : m_component_arrays) {
            array->entity_destroyed(entity_id);
        }
    }

  private:
    std::unordered_map<std::string, std::shared_ptr<IComponentArray>> m_component_arrays;

    template <typename T>
    std::shared_ptr<ComponentArray<T>> get_component_array() const {
        const std::string type_name = typeid(T).name();

        PS_ASSERT(m_component_arrays.contains(type_name), "Component {} is not registered", type_name)
        return std::static_pointer_cast<ComponentArray<T>>(m_component_arrays.find(type_name)->second);
    }
};

} // namespace Phos
