#include "registry.h"

#include "ecs/entity.h"

namespace Phos {

Registry::Registry() {
    for (std::size_t i = 0; i < MAX_NUM_ENTITIES; ++i)
        m_available_ids.push(i);

    m_component_manager = std::make_unique<ComponentManager>();
}

std::shared_ptr<Entity> Registry::create_entity() {
    if (m_available_ids.empty()) {
        PS_FAIL("Maximum number of entities reached");
    }

    const std::size_t id = m_available_ids.front();

    auto entity = std::make_shared<Entity>(id, this);
    // TODO: Add default components

    m_entities[id] = entity;
    return entity;
}

void Registry::destroy_entity(const std::shared_ptr<Entity>& entity) {
    const auto id = entity->id();
    m_available_ids.push(id);

    m_component_manager->entity_destroyed(id);
    m_entities[id] = nullptr;
}

} // namespace Phos