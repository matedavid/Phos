#include "scene.h"

#include "entity.h"

namespace Phos {

Scene::Scene(std::string name) : m_name(std::move(name)) {
    m_registry = std::make_unique<Registry>();
}

Entity Scene::create_entity() {
    auto* entity = new Entity(m_registry->create(), this);
    m_id_to_entity[entity->id()] = entity;

    // Default components
    entity->add_component<TransformComponent>();
    entity->add_component<UUIDComponent>({.uuid = UUID()});
    entity->add_component<RelationshipComponent>();

    m_uuid_to_entity[entity->uuid()] = entity;

    return *entity;
}

void Scene::destroy_entity(Entity entity) {
    m_registry->destroy(entity.id());

    auto* e = m_id_to_entity[entity.id()];
    delete e;

    m_id_to_entity.erase(entity.id());
}

Entity Scene::get_entity_with_uuid(const UUID& uuid) {
    PS_ASSERT(m_uuid_to_entity.contains(uuid), "Scene does not contain entity with uuid: {}", (uint64_t)uuid)

    return *m_uuid_to_entity[uuid];
}

} // namespace Phos
