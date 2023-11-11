#pragma once

#include "scene/scene_definition.h"
#include "scene/entity.h"

namespace Phos {

template <typename... Components>
[[nodiscard]] std::vector<Entity> Scene::get_entities_with() {
    std::vector<std::size_t> ids = m_registry->view<Components...>();

    std::vector<Entity> entities;
    entities.reserve(ids.size());

    for (const auto& entity_id : ids)
        entities.push_back(*m_id_to_entity[entity_id]);

    return entities;
}

} // namespace Phos
