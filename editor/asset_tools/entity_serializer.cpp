#include "entity_serializer.h"

#include <yaml-cpp/yaml.h>

#include "asset_tools/asset_dumping_utils.h"

#include "scene/entity.h"

#include "renderer/mesh.h"
#include "renderer/backend/material.h"

template <typename T>
static void serialize_component_t(YAML::Emitter& out, const T& component);

template <typename T>
void serialize_component(YAML::Emitter& out, const Phos::Entity& entity) {
    if (!entity.has_component<T>())
        return;

    serialize_component_t<T>(out, entity.get_component<T>());
}

#define SERIALIZE_COMPONENT(T) serialize_component<T>(out, entity)

void EntitySerializer::serialize(YAML::Emitter& out, const Phos::Entity& entity) {
    AssetDumpingUtils::AssetDumpingUtils::emit_yaml(out, static_cast<uint64_t>(entity.uuid()));
    out << YAML::BeginMap;

    SERIALIZE_COMPONENT(Phos::NameComponent);
    SERIALIZE_COMPONENT(Phos::RelationshipComponent);
    SERIALIZE_COMPONENT(Phos::TransformComponent);
    SERIALIZE_COMPONENT(Phos::MeshRendererComponent);
    SERIALIZE_COMPONENT(Phos::LightComponent);
    SERIALIZE_COMPONENT(Phos::CameraComponent);
    SERIALIZE_COMPONENT(Phos::ScriptComponent);

    out << YAML::EndMap;
}

//
// Specific serialize_component_t
//

#define SERIALIZE_COMPONENT_T(T) \
    template <>                  \
    void serialize_component_t<T>(YAML::Emitter & out, const T& component)

SERIALIZE_COMPONENT_T(Phos::NameComponent) {
    AssetDumpingUtils::AssetDumpingUtils::emit_yaml(out, "NameComponent", component.name);
}

SERIALIZE_COMPONENT_T(Phos::RelationshipComponent) {
    AssetDumpingUtils::emit_yaml(out, "RelationshipComponent");
    out << YAML::BeginMap;

    if (component.parent.has_value()) {
        AssetDumpingUtils::emit_yaml(out, "parent", (uint64_t)*component.parent);
    }

    if (!component.children.empty()) {
        AssetDumpingUtils::emit_yaml(out, "children");
        out << YAML::BeginSeq;

        for (const auto& child : component.children)
            AssetDumpingUtils::emit_yaml(out, (uint64_t)child);

        out << YAML::EndSeq;
    }

    out << YAML::EndMap;
}

SERIALIZE_COMPONENT_T(Phos::TransformComponent) {
    AssetDumpingUtils::emit_yaml(out, "TransformComponent");
    out << YAML::BeginMap;

    AssetDumpingUtils::emit_yaml(out, "position");
    {
        out << YAML::BeginMap;

        AssetDumpingUtils::emit_yaml(out, "x", component.position.x);
        AssetDumpingUtils::emit_yaml(out, "y", component.position.y);
        AssetDumpingUtils::emit_yaml(out, "z", component.position.z);

        out << YAML::EndMap;
    }

    AssetDumpingUtils::emit_yaml(out, "rotation");
    {
        out << YAML::BeginMap;

        AssetDumpingUtils::emit_yaml(out, "x", component.rotation.x);
        AssetDumpingUtils::emit_yaml(out, "y", component.rotation.y);
        AssetDumpingUtils::emit_yaml(out, "z", component.rotation.z);

        out << YAML::EndMap;
    }

    AssetDumpingUtils::emit_yaml(out, "scale");
    {
        out << YAML::BeginMap;

        AssetDumpingUtils::emit_yaml(out, "x", component.scale.x);
        AssetDumpingUtils::emit_yaml(out, "y", component.scale.y);
        AssetDumpingUtils::emit_yaml(out, "z", component.scale.z);

        out << YAML::EndMap;
    }

    out << YAML::EndMap;
}

SERIALIZE_COMPONENT_T(Phos::MeshRendererComponent) {
    AssetDumpingUtils::emit_yaml(out, "MeshRendererComponent");
    out << YAML::BeginMap;

    AssetDumpingUtils::emit_yaml(out, "mesh", (uint64_t)component.mesh->id);
    AssetDumpingUtils::emit_yaml(out, "material", (uint64_t)component.material->id);

    out << YAML::EndMap;
}

SERIALIZE_COMPONENT_T(Phos::LightComponent) {
    AssetDumpingUtils::emit_yaml(out, "LightComponent");
    out << YAML::BeginMap;

    if (component.type == Phos::Light::Type::Point)
        AssetDumpingUtils::emit_yaml(out, "type", "point");
    else if (component.type == Phos::Light::Type::Directional)
        AssetDumpingUtils::emit_yaml(out, "type", "directional");

    AssetDumpingUtils::emit_yaml(out, "radius", component.radius);

    AssetDumpingUtils::emit_yaml(out, "color");
    {
        out << YAML::BeginMap;

        AssetDumpingUtils::emit_yaml(out, "x", component.color.x);
        AssetDumpingUtils::emit_yaml(out, "y", component.color.y);
        AssetDumpingUtils::emit_yaml(out, "z", component.color.z);
        AssetDumpingUtils::emit_yaml(out, "w", component.color.w);

        out << YAML::EndMap;
    }

    if (component.shadow_type == Phos::Light::ShadowType::None)
        AssetDumpingUtils::emit_yaml(out, "shadow", "none");
    else if (component.shadow_type == Phos::Light::ShadowType::Hard)
        AssetDumpingUtils::emit_yaml(out, "shadow", "hard");

    out << YAML::EndMap;
}

SERIALIZE_COMPONENT_T(Phos::CameraComponent) {
    AssetDumpingUtils::emit_yaml(out, "CameraComponent");
    out << YAML::BeginMap;

    if (component.type == Phos::Camera::Type::Perspective)
        AssetDumpingUtils::emit_yaml(out, "type", "perspective");
    else if (component.type == Phos::Camera::Type::Orthographic)
        AssetDumpingUtils::emit_yaml(out, "type", "orthographic");

    AssetDumpingUtils::emit_yaml(out, "fov", component.fov);
    AssetDumpingUtils::emit_yaml(out, "size", component.size);
    AssetDumpingUtils::emit_yaml(out, "znear", component.znear);
    AssetDumpingUtils::emit_yaml(out, "zfar", component.zfar);
    AssetDumpingUtils::emit_yaml(out, "depth", component.depth);

    out << YAML::EndMap;
}

SERIALIZE_COMPONENT_T(Phos::ScriptComponent) {
    AssetDumpingUtils::emit_yaml(out, "ScriptComponent");
    out << YAML::BeginMap;

    AssetDumpingUtils::emit_yaml(out, "className", component.class_name);
    AssetDumpingUtils::emit_yaml(out, "script", (uint64_t)component.script);

    AssetDumpingUtils::emit_yaml(out, "fields");
    out << YAML::BeginMap;
    {
        for (const auto& [field_name, value] : component.field_values) {
            AssetDumpingUtils::emit_yaml(out, field_name);
            out << YAML::BeginMap;

            std::string type_name;
            if (std::holds_alternative<int>(value)) {
                AssetDumpingUtils::emit_yaml(out, "type", "int");
                AssetDumpingUtils::dump(out, "data", std::get<int>(value));
            } else if (std::holds_alternative<float>(value)) {
                AssetDumpingUtils::emit_yaml(out, "type", "float");
                AssetDumpingUtils::dump(out, "data", std::get<float>(value));
            } else if (std::holds_alternative<glm::vec3>(value)) {
                AssetDumpingUtils::emit_yaml(out, "type", "vec3");
                AssetDumpingUtils::dump_vec3(out, "data", std::get<glm::vec3>(value));
            } else if (std::holds_alternative<std::string>(value)) {
                AssetDumpingUtils::emit_yaml(out, "type", "string");
                AssetDumpingUtils::dump(out, "data", std::get<std::string>(value));
            } else if (std::holds_alternative<Phos::PrefabRef>(value)) {
                AssetDumpingUtils::emit_yaml(out, "type", "prefab");
                AssetDumpingUtils::dump(out, "data", (uint64_t)std::get<Phos::PrefabRef>(value).id);
            } else if (std::holds_alternative<Phos::EntityRef>(value)) {
                AssetDumpingUtils::emit_yaml(out, "type", "entity");
                AssetDumpingUtils::dump(out, "data", (uint64_t)std::get<Phos::EntityRef>(value).id);
            }

            out << YAML::EndMap;
        }
    }
    out << YAML::EndMap;

    out << YAML::EndMap;
}
