#pragma once

#include "core.h"

#include "core/uuid.h"

#include "scene/registry.h"
#include "scene/components.h"

namespace Phos {

// Forward declarations
class Scene;

class Entity {
  public:
    Entity() = default;
    Entity(std::size_t id, Scene* scene) : m_id(id), m_scene(scene) {}
    ~Entity() = default;

    Entity(const Entity& entity) = default;
    Entity& operator=(const Entity& entity) = default;

    template <typename T>
    void add_component(T args = {});

    template <typename T>
    void remove_component();

    template <typename T>
    [[nodiscard]] T& get_component() const;

    void set_parent(const Entity& parent) const;
    void add_child(const Entity& child) const;
    void remove_child(const Entity& child) const;

    [[nodiscard]] UUID uuid() const { return get_component<UUIDComponent>().uuid; }
    [[nodiscard]] std::size_t id() const { return m_id; }

  private:
    std::size_t m_id;
    Scene* m_scene;
};

} // namespace Phos
