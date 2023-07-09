#pragma once

#include "core.h"

namespace Phos {

// Forward declarations
class Entity;

class IComponentArray {
  public:
    virtual ~IComponentArray() = default;
    virtual void entity_destroyed(std::size_t entity_destroyed) = 0;
};

template <typename T>
class ComponentArray : public IComponentArray {
  public:
    ComponentArray() = default;
    ~ComponentArray() override = default;

    void insert_data(std::size_t entity_id, T component) {
        if (m_entity_to_idx.contains(entity_id)) {
            PS_ERROR("Component added to entity {} more than once", entity_id)
            return;
        }

        const auto new_idx = m_size;
        m_entity_to_idx[entity_id] = new_idx;
        m_idx_to_entity[new_idx] = entity_id;
        m_components[new_idx] = component;

        ++m_size;
    }

    void remove_data(std::size_t entity_id) {
        if (!m_entity_to_idx.contains(entity_id)) {
            PS_ERROR("Entity {} does not have component", entity_id)
            return;
        }

        const auto idx_removed = m_entity_to_idx[entity_id];
        const auto idx_last_element = m_size - 1;
        m_components[idx_removed] = m_components[idx_last_element];

        std::size_t last_element_entity_id = m_idx_to_entity[idx_last_element];
        m_entity_to_idx[last_element_entity_id] = idx_removed;
        m_idx_to_entity[idx_removed] = last_element_entity_id;

        m_entity_to_idx.erase(entity_id);
        m_idx_to_entity.erase(idx_last_element);

        --m_size;
    }

    T& get_data(std::size_t entity_id) {
        auto it = m_entity_to_idx.find(entity_id);
        PS_ASSERT(it != m_entity_to_idx.end(), "Entity {} does not have component", entity_id)

        return m_components[it->second];
    }

    void entity_destroyed(std::size_t entity_id) override {
        if (m_entity_to_idx.contains(entity_id))
            remove_data(entity_id);
    }

  private:
    std::array<T, MAX_NUM_ENTITIES> m_components;
    uint32_t m_size = 0;

    std::unordered_map<std::size_t, uint32_t> m_entity_to_idx;
    std::unordered_map<uint32_t, std::size_t> m_idx_to_entity;
};

} // namespace Phos
