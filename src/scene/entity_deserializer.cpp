#include "entity_deserializer.h"

#include <yaml-cpp/yaml.h>

#include "asset/asset_manager.h"

#include "renderer/mesh.h"
#include "renderer/backend/material.h"

namespace Phos {

template <typename T>
void deserialize_component_t(const YAML::Node& node, Entity& entity, [[maybe_unused]] AssetManagerBase* asset_manager);

template <typename T>
void deserialize_component(const YAML::Node& node,
                           const std::string& name,
                           Entity& entity,
                           AssetManagerBase* asset_manager) {
    if (!node[name])
        return;

    deserialize_component_t<T>(node[name], entity, asset_manager);
}

#define DESERIALIZE_COMPONENT(T, name) deserialize_component<T>(node, name, entity, asset_manager)

Entity EntityDeserializer::deserialize(const YAML::Node& node,
                                       const UUID& asset_id,
                                       const std::shared_ptr<Scene>& scene,
                                       AssetManagerBase* asset_manager) {
    auto entity = scene->create_entity(asset_id);

    DESERIALIZE_COMPONENT(NameComponent, "NameComponent");
    DESERIALIZE_COMPONENT(RelationshipComponent, "RelationshipComponent");
    DESERIALIZE_COMPONENT(TransformComponent, "TransformComponent");
    DESERIALIZE_COMPONENT(MeshRendererComponent, "MeshRendererComponent");
    DESERIALIZE_COMPONENT(LightComponent, "LightComponent");
    DESERIALIZE_COMPONENT(CameraComponent, "CameraComponent");

    return entity;
}

//
// Specific deserialize_component_t
//

#define DESERIALIZE_COMPONENT_T(Type)   \
    template <>                         \
    void deserialize_component_t<Type>( \
        const YAML::Node& node, Entity& entity, [[maybe_unused]] AssetManagerBase* asset_manager)

DESERIALIZE_COMPONENT_T(NameComponent) {
    entity.get_component<NameComponent>().name = node.as<std::string>();
}

DESERIALIZE_COMPONENT_T(RelationshipComponent) {
    if (node["parent"]) {
        const auto parent_uuid = node["parent"].as<uint64_t>();
        entity.get_component<RelationshipComponent>().parent = UUID(parent_uuid);
    }

    if (node["children"]) {
        for (const auto child_uuid : node["children"]) {
            entity.get_component<RelationshipComponent>().children.emplace_back(child_uuid.as<uint64_t>());
        }
    }
}

DESERIALIZE_COMPONENT_T(TransformComponent) {
    auto& transform = entity.get_component<TransformComponent>();

    const auto& position = node["position"];
    transform.position = glm::vec3(position["x"].as<float>(), position["y"].as<float>(), position["z"].as<float>());

    const auto& rotation = node["rotation"];
    transform.rotation = glm::vec3(rotation["x"].as<float>(), rotation["y"].as<float>(), rotation["z"].as<float>());

    const auto& scale = node["scale"];
    transform.scale = glm::vec3(scale["x"].as<float>(), scale["y"].as<float>(), scale["z"].as<float>());
}

DESERIALIZE_COMPONENT_T(MeshRendererComponent) {
    const auto mesh_uuid = UUID(node["mesh"].as<uint64_t>());
    const auto material_uuid = UUID(node["material"].as<uint64_t>());

    entity.add_component<MeshRendererComponent>({
        .mesh = asset_manager->load_by_id_type<Mesh>(mesh_uuid),
        .material = asset_manager->load_by_id_type<Material>(material_uuid),
    });
}

DESERIALIZE_COMPONENT_T(LightComponent) {
    const auto light_type_str = node["type"].as<std::string>();
    Light::Type light_type;
    if (light_type_str == "point")
        light_type = Light::Type::Point;
    else if (light_type_str == "directional")
        light_type = Light::Type::Directional;

    const auto radius = node["radius"].as<float>();

    const auto color_node = node["color"];
    const glm::vec4 color = {
        color_node["x"].as<uint32_t>(),
        color_node["y"].as<uint32_t>(),
        color_node["z"].as<uint32_t>(),
        color_node["w"].as<uint32_t>(),
    };

    const auto shadow_type_str = node["shadow"].as<std::string>();
    Light::ShadowType shadow_type;
    if (shadow_type_str == "none")
        shadow_type = Light::ShadowType::None;
    else if (shadow_type_str == "hard")
        shadow_type = Light::ShadowType::Hard;

    entity.add_component<LightComponent>({
        .type = light_type,
        .radius = radius,
        .color = color,
        .shadow_type = shadow_type,
    });
}

DESERIALIZE_COMPONENT_T(CameraComponent) {
    const auto camera_type_str = node["type"].as<std::string>();
    Camera::Type camera_type;
    if (camera_type_str == "perspective")
        camera_type = Camera::Type::Perspective;
    else if (camera_type_str == "orthographic")
        camera_type = Camera::Type::Orthographic;

    const auto fov = node["fov"].as<float>();
    const auto size = node["size"].as<float>();
    const auto znear = node["znear"].as<float>();
    const auto zfar = node["zfar"].as<float>();
    const auto depth = node["depth"].as<int32_t>();

    entity.add_component<CameraComponent>({
        .type = camera_type,
        .fov = fov,
        .size = size,
        .znear = znear,
        .zfar = zfar,
        .depth = depth,
    });
}

} // namespace Phos
