#pragma once

#include <algorithm>
#include <ranges>

namespace Phos {

template <typename T>
void Entity::add_component(T args) {
    m_scene->m_registry->add_component<T>(m_id, args);
}

template <typename T>
void Entity::remove_component() {
    m_scene->m_registry->remove_component<T>(m_id);
}

template <typename T>
T& Entity::get_component() const {
    return m_scene->m_registry->get_component<T>(m_id);
}

void Entity::set_parent(const Entity& parent) const {
    auto& relationship = get_component<RelationshipComponent>();

    if (relationship.parent) {
        const auto& current_parent = m_scene->get_entity_with_uuid(relationship.parent.value());
        current_parent.remove_child(*this);
    }

    relationship.parent = parent.uuid();
    parent.add_child(*this);
}

void Entity::add_child(const Entity& child) const {
    auto& relationship = get_component<RelationshipComponent>();

    if (std::ranges::find(relationship.children, child.uuid()) != relationship.children.end()) {
        PS_WARNING("Entity {} already contains child {}", (uint64_t)uuid(), (uint64_t)child.uuid());
        return;
    }

    relationship.children.push_back(child.uuid());
}

void Entity::remove_child(const Entity& child) const {
    auto& relationship = get_component<RelationshipComponent>();

    if (std::ranges::find(relationship.children, child.uuid()) == relationship.children.end()) {
        PS_ERROR("Entity {} does not contain child: {}", (uint64_t)uuid(), (uint64_t)child.uuid());
        return;
    }

    std::ranges::remove(relationship.children, child.uuid());
}

} // namespace Phos