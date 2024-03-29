#pragma once

#include <ranges>

#include "core/uuid.h"
#include "utility/logging.h"

#include "scene/registry.h"
#include "scene/components.h"
#include "scene/scene_definition.h"

namespace Phos {

class Entity {
  public:
    Entity() = default;
    Entity(std::size_t id, Scene* scene) : m_id(id), m_scene(scene) {}
    ~Entity() = default;

    Entity(const Entity& entity) = default;
    Entity& operator=(const Entity& entity) = default;

    bool operator==(const Entity& entity) const { return uuid() == entity.uuid(); }
    bool operator!=(const Entity& entity) const { return !(*this == entity); }

    template <typename T>
    void add_component(T args = {}) {
        m_scene->m_registry->add_component<T>(m_id, args);
    }

    template <typename T>
    void remove_component() {
        m_scene->m_registry->remove_component<T>(m_id);
    }

    template <typename T>
    [[nodiscard]] T& get_component() const {
        return m_scene->m_registry->get_component<T>(m_id);
    }

    template <typename T>
    [[nodiscard]] bool has_component() const {
        return m_scene->m_registry->has_component<T>(m_id);
    }

    void set_parent(const Entity& parent) const {
        auto& relationship = get_component<RelationshipComponent>();

        if (relationship.parent) {
            const auto& current_parent = m_scene->get_entity_with_uuid(relationship.parent.value());
            current_parent.remove_child(*this);
        }

        relationship.parent = parent.uuid();
        parent.add_child(*this);
    }

    void add_child(const Entity& child) const {
        auto& relationship = get_component<RelationshipComponent>();

        if (std::ranges::find(relationship.children, child.uuid()) != relationship.children.end()) {
            PHOS_LOG_WARNING("Entity {} already contains child {}",
                             static_cast<uint64_t>(uuid()),
                             static_cast<uint64_t>(child.uuid()));
            return;
        }

        relationship.children.push_back(child.uuid());
        child.get_component<RelationshipComponent>().parent = uuid();
    }

    void remove_child(const Entity& child) const {
        auto& relationship = get_component<RelationshipComponent>();

        if (std::ranges::find(relationship.children, child.uuid()) == relationship.children.end()) {
            PHOS_LOG_ERROR("Entity {} does not contain child: {}",
                           static_cast<uint64_t>(uuid()),
                           static_cast<uint64_t>(child.uuid()));
            return;
        }

        std::erase(relationship.children, child.uuid());
    }

    [[nodiscard]] UUID uuid() const { return get_component<UUIDComponent>().uuid; }
    [[nodiscard]] std::size_t id() const { return m_id; }

  private:
    std::size_t m_id;
    Scene* m_scene;
};

} // namespace Phos
