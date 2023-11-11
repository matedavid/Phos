#include "scene_serializer.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

#include "asset/asset_parsing_utils.h"

#include "scene/entity.h"
#include "scene/scene.h"

#include "renderer/mesh.h"
#include "renderer/backend/material.h"
#include "renderer/backend/cubemap.h"

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
    emit_yaml(out, "assetType", "scene");
    emit_yaml(out, "name", scene->name());
    emit_yaml(out, "id", (uint64_t)scene->id);

    // Emit config
    const auto& config = scene->config();

    emit_yaml(out, "config");
    out << YAML::BeginMap;

    emit_yaml(out, "bloomConfig");
    {
        out << YAML::BeginMap;

        emit_yaml(out, "enabled", config.bloom_config.enabled);
        emit_yaml(out, "threshold", config.bloom_config.threshold);

        out << YAML::EndMap;
    }

    emit_yaml(out, "environmentConfig");
    {
        out << YAML::BeginMap;

        emit_yaml(out, "skybox", (uint64_t)config.environment_config.skybox->id);

        out << YAML::EndMap;
    }

    out << YAML::EndMap;

    // Emit entities
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
    SERIALIZE_COMPONENT(ScriptComponent);

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

template <>
void serialize_component_t<ScriptComponent>(YAML::Emitter& out, const ScriptComponent& component) {
    emit_yaml(out, "ScriptComponent");
    out << YAML::BeginMap;

    emit_yaml(out, "className", component.class_name);
    emit_yaml(out, "script", (uint64_t)component.script);

    emit_yaml(out, "fields");
    out << YAML::BeginMap;
    {
        for (const auto& [field_name, value] : component.field_values) {
            emit_yaml(out, field_name);
            out << YAML::BeginMap;

            std::string type_name;
            if (std::holds_alternative<int>(value)) {
                emit_yaml(out, "type", "int");
                AssetParsingUtils::dump(out, "data", std::get<int>(value));
            } else if (std::holds_alternative<float>(value)) {
                emit_yaml(out, "type", "float");
                AssetParsingUtils::dump(out, "data", std::get<float>(value));
            } else if (std::holds_alternative<glm::vec3>(value)) {
                emit_yaml(out, "type", "vec3");
                AssetParsingUtils::dump_vec3(out, "data", std::get<glm::vec3>(value));
            } else if (std::holds_alternative<std::string>(value)) {
                emit_yaml(out, "type", "string");
                AssetParsingUtils::dump(out, "data", std::get<std::string>(value));
            } else if (std::holds_alternative<PrefabRef>(value)) {
                emit_yaml(out, "type", "prefab");
                AssetParsingUtils::dump(out, "data", (uint64_t)std::get<PrefabRef>(value).id);
            } else if (std::holds_alternative<EntityRef>(value)) {
                emit_yaml(out, "type", "entity");
                AssetParsingUtils::dump(out, "data", (uint64_t)std::get<EntityRef>(value).id);
            }

            out << YAML::EndMap;
        }
    }
    out << YAML::EndMap;

    out << YAML::EndMap;
}

} // namespace Phos
