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
    m_registry->register_component<Phos::ScriptComponent>();
}

#define COPY_COMPONENT(T)                              \
    if (entity.has_component<T>()) {                   \
        const auto c = entity.get_component<T>();      \
        if (copy_entity.has_component<T>()) {          \
            auto& cc = copy_entity.get_component<T>(); \
            cc = c;                                    \
        } else {                                       \
            copy_entity.add_component<T>(c);           \
        }                                              \
    }

Scene::Scene(Scene& other) : Scene(other.name() + "Copy") {
    // @TODO: Probably very slow, look into more efficient way
    // Leaving for the moment because there will not be many copies
    for (const auto& entity : other.get_all_entities()) {
        auto copy_entity = create_entity(entity.uuid());

        COPY_COMPONENT(NameComponent);
        COPY_COMPONENT(RelationshipComponent);
        COPY_COMPONENT(TransformComponent);
        COPY_COMPONENT(LightComponent);
        COPY_COMPONENT(MeshRendererComponent);
        COPY_COMPONENT(CameraComponent);
        COPY_COMPONENT(ScriptComponent);
    }

    m_renderer_config = other.m_renderer_config;
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
    destroy_entity_r(entity);
}

void Scene::destroy_entity_r(Phos::Entity entity) {
    auto* e = m_id_to_entity[entity.id()];

    // Remove entity from parent children
    const auto relationship = entity.get_component<RelationshipComponent>();
    if (relationship.parent) {
        const auto parent = get_entity_with_uuid(*relationship.parent);
        parent.remove_child(entity);
    }

    // Remove child entities
    for (const auto& child_id : relationship.children) {
        const auto child = get_entity_with_uuid(child_id);
        destroy_entity_r(child);
    }

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
    for (const auto& [_, entity] : m_uuid_to_entity) {
        entities[i++] = *entity;
    }

    return entities;
}

} // namespace Phos
