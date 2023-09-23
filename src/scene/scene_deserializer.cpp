#include "scene_deserializer.h"

#include <yaml-cpp/yaml.h>

#include "scene/scene.h"
#include "scene/entity.h"

#include "asset/asset_manager.h"

#include "renderer/mesh.h"
#include "renderer/backend/material.h"
#include "renderer/backend/shader.h"

namespace Phos {

// TODO: Super ugly, but only temporal
static std::shared_ptr<AssetManagerBase> m_asset_manager = nullptr;

std::shared_ptr<Scene> SceneDeserializer::deserialize(const std::string& path,
                                                      const std::shared_ptr<AssetManagerBase>& asset_manager) {
    m_asset_manager = asset_manager;

    const YAML::Node node = YAML::LoadFile(path);

    const auto name = node["name"].as<std::string>();
    auto scene = std::make_shared<Scene>(name);

    const auto entities = node["entities"];
    for (const auto& it : entities) {
        const auto uuid = UUID(it.first.as<std::size_t>());

        auto entity = scene->create_entity(uuid);
        deserialize_entity(entities[it.first], entity);
    }

    m_asset_manager = nullptr;

    return scene;
}

template <typename T>
void deserialize_component_t(const YAML::Node& node, Entity& entity);

template <typename T>
void deserialize_component(const YAML::Node& node, const std::string& name, Entity& entity) {
    if (!node[name])
        return;

    deserialize_component_t<T>(node[name], entity);
}

#define DESERIALIZE_COMPONENT(T, name) deserialize_component<T>(node, name, entity)

void SceneDeserializer::deserialize_entity(const YAML::Node& node, Entity& entity) {
    DESERIALIZE_COMPONENT(NameComponent, "NameComponent");
    DESERIALIZE_COMPONENT(RelationshipComponent, "RelationshipComponent");
    DESERIALIZE_COMPONENT(TransformComponent, "TransformComponent");
    DESERIALIZE_COMPONENT(MeshRendererComponent, "MeshRendererComponent");
    DESERIALIZE_COMPONENT(LightComponent, "LightComponent");
    DESERIALIZE_COMPONENT(CameraComponent, "CameraComponent");
}

//
// Specific deserialize_component_t
//

template <>
void deserialize_component_t<NameComponent>(const YAML::Node& node, Entity& entity) {
    entity.get_component<NameComponent>().name = node.as<std::string>();
}

template <>
void deserialize_component_t<RelationshipComponent>(const YAML::Node& node, Entity& entity) {
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

template <>
void deserialize_component_t<TransformComponent>(const YAML::Node& node, Entity& entity) {
    auto& transform = entity.get_component<TransformComponent>();

    const auto& position = node["position"];
    transform.position = glm::vec3(position["x"].as<float>(), position["y"].as<float>(), position["z"].as<float>());

    const auto& rotation = node["rotation"];
    transform.rotation = glm::vec3(rotation["x"].as<float>(), rotation["y"].as<float>(), rotation["z"].as<float>());

    const auto& scale = node["scale"];
    transform.scale = glm::vec3(scale["x"].as<float>(), scale["y"].as<float>(), scale["z"].as<float>());
}

template <>
void deserialize_component_t<MeshRendererComponent>(const YAML::Node& node, Entity& entity) {
    const auto mesh_uuid = UUID(node["mesh"].as<uint64_t>());
    const auto material_uuid = UUID(node["material"].as<uint64_t>());

    entity.add_component<MeshRendererComponent>({
        .mesh = m_asset_manager->load_by_id_type<Mesh>(mesh_uuid),
        .material = m_asset_manager->load_by_id_type<Material>(material_uuid),
    });
}

template <>
void deserialize_component_t<LightComponent>(const YAML::Node& node, Entity& entity) {
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

template <>
void deserialize_component_t<CameraComponent>(const YAML::Node& node, Entity& entity) {
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
