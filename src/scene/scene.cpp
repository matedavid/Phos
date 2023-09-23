#include "scene.h"

#include <utility>

#include "scene/entity.h"

namespace Phos {

Scene::Scene(std::string name) : m_name(std::move(name)) {
    m_registry = std::make_unique<Registry>();

    // Register default components
    m_registry->register_component<Phos::UUIDComponent>();
    m_registry->register_component<Phos::RelationshipComponent>();
    m_registry->register_component<Phos::NameComponent>();

    m_registry->register_component<Phos::TransformComponent>();
    m_registry->register_component<Phos::MeshRendererComponent>();
    m_registry->register_component<Phos::LightComponent>();
    m_registry->register_component<Phos::CameraComponent>();
}

Scene::~Scene() {
    for (auto& [_, entity] : m_id_to_entity) {
        delete entity;
    }
}

Entity Scene::create_entity() {
    const auto num_entities = m_uuid_to_entity.size();
    const std::string default_name = "Entity " + std::to_string(num_entities);
    return create_entity(default_name, UUID());
}

Entity Scene::create_entity(const UUID uuid) {
    const auto num_entities = m_uuid_to_entity.size();
    const std::string default_name = "Entity " + std::to_string(num_entities);
    return create_entity(default_name, uuid);
}

Entity Scene::create_entity(const std::string& name, const UUID uuid) {
    auto* entity = new Entity(m_registry->create(), this);
    m_id_to_entity[entity->id()] = entity;

    // Default components
    entity->add_component<TransformComponent>();
    entity->add_component<UUIDComponent>({.uuid = uuid});
    entity->add_component<RelationshipComponent>();
    entity->add_component<NameComponent>({.name = name});

    m_uuid_to_entity[entity->uuid()] = entity;

    return *entity;
}

void Scene::destroy_entity(Entity entity) {
    auto* e = m_id_to_entity[entity.id()];

    m_id_to_entity.erase(entity.id());
    m_uuid_to_entity.erase(entity.uuid());

    m_registry->destroy(entity.id());

    delete e;
}

Entity Scene::get_entity_with_uuid(const UUID& uuid) {
    PS_ASSERT(m_uuid_to_entity.contains(uuid), "Scene does not contain entity with uuid: {}", (uint64_t)uuid)

    return *m_uuid_to_entity[uuid];
}

std::vector<Entity> Scene::get_all_entities() const {
    std::vector<Entity> entities(m_uuid_to_entity.size());

    uint32_t i = 0;
    for (auto& [_, entity] : m_uuid_to_entity) {
        entities[i++] = *entity;
    }

    return entities;
}

} // namespace Phos
