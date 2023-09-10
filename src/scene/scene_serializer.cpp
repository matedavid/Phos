#include "scene_serializer.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

#include "scene/entity.h"
#include "scene/scene.h"

#include "renderer/mesh.h"
#include "renderer/backend/material.h"

namespace Phos {

template <typename F>
void emit_yaml(YAML::Emitter& out, const F& key) {
    out << YAML::Key << key;
    out << YAML::Value;
}

template <typename F, typename S>
void emit_yaml(YAML::Emitter& out, const F& key, const S& value) {
    out << YAML::Key << key;
    out << YAML::Value << value;
}

void SceneSerializer::serialize(const std::shared_ptr<Scene>& scene, const std::string& path) {
    const auto& entities = scene->get_all_entities();

    YAML::Emitter out;
    out << YAML::BeginMap;
    emit_yaml(out, "name", scene->name());

    emit_yaml(out, "entities");
    out << YAML::BeginMap;

    for (const auto& entity : entities)
        serialize_entity(out, entity);

    out << YAML::EndMap << YAML::EndMap;

    std::ofstream file(path);
    file << out.c_str();
}

template <typename T>
static void serialize_component_t(YAML::Emitter& out, const T& component);

template <typename T>
void serialize_component(YAML::Emitter& out, const Entity& entity) {
    if (!entity.has_component<T>())
        return;

    serialize_component_t<T>(out, entity.get_component<T>());
}

#define SERIALIZE_COMPONENT(T) serialize_component<T>(out, entity)

void SceneSerializer::serialize_entity(YAML::Emitter& out, const Entity& entity) {
    emit_yaml(out, (uint64_t)entity.uuid());
    out << YAML::BeginMap;

    SERIALIZE_COMPONENT(NameComponent);
    SERIALIZE_COMPONENT(RelationshipComponent);
    SERIALIZE_COMPONENT(TransformComponent);
    SERIALIZE_COMPONENT(MeshRendererComponent);
    SERIALIZE_COMPONENT(LightComponent);
    SERIALIZE_COMPONENT(CameraComponent);

    out << YAML::EndMap;
}

//
// Specific serialize_component_t
//

template <>
void serialize_component_t<NameComponent>(YAML::Emitter& out, const NameComponent& component) {
    emit_yaml(out, "NameComponent", component.name);
}

template <>
void serialize_component_t<RelationshipComponent>(YAML::Emitter& out, const RelationshipComponent& component) {
    emit_yaml(out, "RelationshipComponent");
    out << YAML::BeginMap;

    if (component.parent.has_value()) {
        emit_yaml(out, "parent", (uint64_t)*component.parent);
    }

    if (!component.children.empty()) {
        emit_yaml(out, "children");
        out << YAML::BeginSeq;

        for (const auto& child : component.children)
            emit_yaml(out, (uint64_t)child);

        out << YAML::EndSeq;
    }

    out << YAML::EndMap;
}

template <>
void serialize_component_t<TransformComponent>(YAML::Emitter& out, const TransformComponent& component) {
    emit_yaml(out, "TransformComponent");
    out << YAML::BeginMap;

    emit_yaml(out, "position");
    {
        out << YAML::BeginMap;

        emit_yaml(out, "x", component.position.x);
        emit_yaml(out, "y", component.position.y);
        emit_yaml(out, "z", component.position.z);

        out << YAML::EndMap;
    }

    emit_yaml(out, "rotation");
    {
        out << YAML::BeginMap;

        emit_yaml(out, "x", component.rotation.x);
        emit_yaml(out, "y", component.rotation.y);
        emit_yaml(out, "z", component.rotation.z);

        out << YAML::EndMap;
    }

    emit_yaml(out, "scale");
    {
        out << YAML::BeginMap;

        emit_yaml(out, "x", component.scale.x);
        emit_yaml(out, "y", component.scale.y);
        emit_yaml(out, "z", component.scale.z);

        out << YAML::EndMap;
    }

    out << YAML::EndMap;
}

template <>
void serialize_component_t<MeshRendererComponent>(YAML::Emitter& out, const MeshRendererComponent& component) {
    emit_yaml(out, "MeshRendererComponent");
    out << YAML::BeginMap;

    emit_yaml(out, "mesh", (uint64_t)component.mesh->id);
    emit_yaml(out, "material", (uint64_t)component.material->id);

    out << YAML::EndMap;
}

template <>
void serialize_component_t<LightComponent>(YAML::Emitter& out, const LightComponent& component) {
    emit_yaml(out, "LightComponent");
    out << YAML::BeginMap;

    if (component.type == Light::Type::Point)
        emit_yaml(out, "type", "point");
    else if (component.type == Light::Type::Directional)
        emit_yaml(out, "type", "directional");

    emit_yaml(out, "radius", component.radius);

    emit_yaml(out, "color");
    {
        out << YAML::BeginMap;

        emit_yaml(out, "x", component.color.x);
        emit_yaml(out, "y", component.color.y);
        emit_yaml(out, "z", component.color.z);
        emit_yaml(out, "w", component.color.w);

        out << YAML::EndMap;
    }

    if (component.shadow_type == Light::ShadowType::None)
        emit_yaml(out, "shadow", "none");
    else if (component.shadow_type == Light::ShadowType::Hard)
        emit_yaml(out, "shadow", "hard");

    out << YAML::EndMap;
}

template <>
void serialize_component_t<CameraComponent>(YAML::Emitter& out, const CameraComponent& component) {
    emit_yaml(out, "CameraComponent");
    out << YAML::BeginMap;

    if (component.type == Camera::Type::Perspective)
        emit_yaml(out, "type", "perspective");
    else if (component.type == Camera::Type::Orthographic)
        emit_yaml(out, "type", "orthographic");

    emit_yaml(out, "fov", component.fov);
    emit_yaml(out, "size", component.size);
    emit_yaml(out, "znear", component.znear);
    emit_yaml(out, "zfar", component.zfar);
    emit_yaml(out, "depth", component.depth);

    out << YAML::EndMap;
}

} // namespace Phos
