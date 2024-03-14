#include "entity_serializer.h"

#include "scene/entity.h"

#include "renderer/mesh.h"
#include "renderer/backend/material.h"

template <typename T>
static void serialize_component_t(const T& component, AssetBuilder& builder);

template <typename T>
void serialize_component(const Phos::Entity& entity, AssetBuilder& builder) {
    if (!entity.has_component<T>())
        return;

    serialize_component_t<T>(entity.get_component<T>(), builder);
}

#define SERIALIZE_COMPONENT(T) serialize_component<T>(entity, builder)

AssetBuilder EntitySerializer::serialize(const Phos::Entity& entity) {
    auto builder = AssetBuilder();

    SERIALIZE_COMPONENT(Phos::NameComponent);
    SERIALIZE_COMPONENT(Phos::RelationshipComponent);
    SERIALIZE_COMPONENT(Phos::TransformComponent);
    SERIALIZE_COMPONENT(Phos::MeshRendererComponent);
    SERIALIZE_COMPONENT(Phos::LightComponent);
    SERIALIZE_COMPONENT(Phos::CameraComponent);
    SERIALIZE_COMPONENT(Phos::ScriptComponent);

    return builder;
}

//
// Specific serialize_component_t
//

#define SERIALIZE_COMPONENT_T(T) \
    template <>                  \
    void serialize_component_t<T>(const T& component, AssetBuilder& builder)

SERIALIZE_COMPONENT_T(Phos::NameComponent) {
    builder.dump("NameComponent", component.name);
}

SERIALIZE_COMPONENT_T(Phos::RelationshipComponent) {
    auto component_builder = AssetBuilder();

    if (component.parent.has_value()) {
        component_builder.dump("parent", *component.parent);
    }

    if (!component.children.empty()) {
        auto children_list = std::vector<uint64_t>();

        for (const auto& child : component.children)
            children_list.push_back(static_cast<uint64_t>(child));

        component_builder.dump("children", children_list);
    }

    builder.dump("RelationshipComponent", component_builder);
}

SERIALIZE_COMPONENT_T(Phos::TransformComponent) {
    auto component_builder = AssetBuilder();

    // Position
    component_builder.dump("position", component.position);
    // Rotation
    component_builder.dump("rotation", glm::eulerAngles(component.rotation));
    // Scale
    component_builder.dump("scale", component.scale);

    builder.dump("TransformComponent", component_builder);
}

SERIALIZE_COMPONENT_T(Phos::MeshRendererComponent) {
    auto component_builder = AssetBuilder();

    const auto mesh_id = component.mesh == nullptr ? Phos::UUID(0) : component.mesh->id;
    component_builder.dump("mesh", mesh_id);

    const auto material_id = component.material == nullptr ? Phos::UUID(0) : component.material->id;
    component_builder.dump("material", material_id);

    builder.dump("MeshRendererComponent", component_builder);
}

SERIALIZE_COMPONENT_T(Phos::LightComponent) {
    auto component_builder = AssetBuilder();

    switch (component.type) {
    case Phos::Light::Type::Point:
        component_builder.dump("type", "point");
        break;
    case Phos::Light::Type::Directional:
        component_builder.dump("type", "directional");
        break;
    }

    component_builder.dump("intensity", component.intensity);
    component_builder.dump("color", component.color);

    switch (component.shadow_type) {
    case Phos::Light::ShadowType::None:
        component_builder.dump("shadow", "none");
        break;
    case Phos::Light::ShadowType::Hard:
        component_builder.dump("shadow", "hard");
        break;
    }

    builder.dump("LightComponent", component_builder);
}

SERIALIZE_COMPONENT_T(Phos::CameraComponent) {
    auto component_builder = AssetBuilder();

    switch (component.type) {
    case Phos::Camera::Type::Perspective:
        component_builder.dump("type", "perspective");
        break;
    case Phos::Camera::Type::Orthographic:
        component_builder.dump("type", "orthographic");
        break;
    }

    component_builder.dump("fov", component.fov);
    component_builder.dump("size", component.size);
    component_builder.dump("znear", component.znear);
    component_builder.dump("zfar", component.zfar);
    component_builder.dump("depth", component.depth);

    builder.dump("CameraComponent", component_builder);
}

SERIALIZE_COMPONENT_T(Phos::ScriptComponent) {
    auto component_builder = AssetBuilder();

    component_builder.dump("className", component.class_name);
    component_builder.dump("script", component.script);

    {
        auto fields_builder = AssetBuilder();

        for (const auto& [field_name, value] : component.field_values) {
            auto field_builder = AssetBuilder();

            if (std::holds_alternative<int>(value)) {
                field_builder.dump("type", "int");
                field_builder.dump("data", std::get<int>(value));
            } else if (std::holds_alternative<float>(value)) {
                field_builder.dump("type", "float");
                field_builder.dump("data", std::get<float>(value));
            } else if (std::holds_alternative<glm::vec3>(value)) {
                field_builder.dump("type", "vec3");
                field_builder.dump("data", std::get<glm::vec3>(value));
            } else if (std::holds_alternative<std::string>(value)) {
                field_builder.dump("type", "string");
                field_builder.dump("data", std::get<std::string>(value));
            } else if (std::holds_alternative<Phos::PrefabRef>(value)) {
                field_builder.dump("type", "prefab");
                field_builder.dump("data", std::get<Phos::PrefabRef>(value).id);
            } else if (std::holds_alternative<Phos::EntityRef>(value)) {
                field_builder.dump("type", "entity");
                field_builder.dump("data", std::get<Phos::EntityRef>(value).id);
            }

            fields_builder.dump(field_name, field_builder);
        }

        component_builder.dump("fields", fields_builder);
    }

    builder.dump("ScriptComponent", component_builder);
}
