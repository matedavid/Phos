#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <optional>

#include "core/uuid.h"
#include "renderer/light.h"
#include "renderer/camera.h"
#include "scripting/class_field.h"

namespace Phos {

// Forward declarations
class StaticMesh;
class Material;
class ClassInstanceHandle;

struct UUIDComponent {
    UUID uuid;
};

struct RelationshipComponent {
    std::optional<UUID> parent;
    std::vector<UUID> children;
};

struct NameComponent {
    std::string name;
};

struct TransformComponent {
    glm::vec3 position{};
    glm::vec3 rotation{}; // Euler angles
    glm::vec3 scale{1.0f};
};

struct LightComponent {
    Light::Type type = Light::Type::Point;

    glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;

    Light::ShadowType shadow_type = Light::ShadowType::Hard;
};

struct MeshRendererComponent {
    std::shared_ptr<StaticMesh> mesh;
    std::shared_ptr<Material> material;
};

struct CameraComponent {
    Camera::Type type = Camera::Type::Perspective;

    float fov = 90.0f; // Only Perspective Camera (degrees)
    float size = 5.0f; // Only Orthographic Camera

    float znear = 0.01f;
    float zfar = 100.0f;

    int32_t depth = 0;
};

struct ScriptComponent {
    std::string class_name;
    UUID script{0};
    std::unordered_map<std::string, ClassField::Value> field_values;
};

} // namespace Phos
