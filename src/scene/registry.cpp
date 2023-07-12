#include "registry.h"

namespace Phos {

Registry::Registry() {
    for (std::size_t i = 0; i < MAX_NUM_ENTITIES; ++i)
        m_available_ids.push(i);

    m_component_manager = std::make_unique<ComponentManager>();
}

std::size_t Registry::create() {
    if (m_available_ids.empty()) {
        PS_FAIL("Maximum number of entities reached");
    }

    auto id = m_available_ids.front();
    m_available_ids.pop();

    return id;
}

void Registry::destroy(std::size_t entity) {
    m_available_ids.push(entity);
    m_component_manager->entity_destroyed(entity);
}

} // namespace Phos