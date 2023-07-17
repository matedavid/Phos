#pragma once

#include <glm/glm.hpp>
#include <optional>

#include "core/uuid.h"

namespace Phos {

// Forward declarations
class Mesh;
class Material;

struct UUIDComponent {
    UUID uuid;
};

struct RelationshipComponent {
    std::optional<UUID> parent;
    std::vector<UUID> children;
};

struct TransformComponent {
    glm::vec3 position{};
    glm::vec3 rotation{};
    glm::vec3 scale{1.0f};
};

struct LightComponent {
    enum class Type {
        Point,
        Directional,
    };

    Type light_type = Type::Point;

    float radius = 10.0f; // Only Point lights
    glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
};

struct MeshRendererComponent {
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> material;
};

} // namespace Phos
