#pragma once

#include "core.h"

#include <glm/glm.hpp>

namespace Phos {

class Light {
  public:
    enum class Type {
        Point,
        Directional,
    };

    virtual ~Light() = default;
    [[nodiscard]] virtual Type type() const = 0;
};

class PointLight : public Light {
  public:
    PointLight() = default;
    explicit PointLight(glm::vec3 position, glm::vec4 color);
    ~PointLight() override = default;

    [[nodiscard]] Type type() const override { return Type::Point; }

    glm::vec3 position{};
    glm::vec4 color{};
};

class DirectionalLight : public Light {
  public:
    DirectionalLight() = default;
    explicit DirectionalLight(glm::vec3 direction, glm::vec4 color);
    ~DirectionalLight() override = default;

    [[nodiscard]] Type type() const override { return Type::Directional; }

    glm::vec3 direction{};
    glm::vec4 color{};
};

} // namespace Phos
