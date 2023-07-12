#include "scene.h"

namespace Phos {

Scene::Scene(std::string name) : m_name(std::move(name)) {
    m_registry = std::make_unique<Registry>();
}

Entity Scene::create_entity() {
    Entity entity = Entity(m_registry->create(), m_registry.get());
    m_id_to_entity[entity.id()] = entity;

    // Default components
    entity.add_component<TransformComponent>();

    return entity;
}

void Scene::destroy_entity(Entity entity) {
    m_registry->destroy(entity.id());
    m_id_to_entity.erase(entity.id());
}

} // namespace Phos
