#pragma once

#include <glm/glm.hpp>

namespace Phos {

class Light {
  public:
    enum class Type {
        Point,
        Directional,
    };

    enum class ShadowType {
        None,
        // TODO: Soft shadows not implemented
        // Soft
        Hard,
    };

    virtual ~Light() = default;
    [[nodiscard]] virtual Type type() const = 0;

    glm::vec3 position{};
    glm::vec4 color{};
    ShadowType shadow_type = ShadowType::None;

  protected:
    Light() = default;
    Light(glm::vec3 position, glm::vec4 color);
};

class PointLight : public Light {
  public:
    PointLight() = default;
    explicit PointLight(glm::vec3 position, glm::vec4 color);
    ~PointLight() override = default;

    [[nodiscard]] Type type() const override { return Type::Point; }
};

class DirectionalLight : public Light {
  public:
    DirectionalLight() = default;
    explicit DirectionalLight(glm::vec3 position, glm::vec3 direction, glm::vec4 color);
    ~DirectionalLight() override = default;

    [[nodiscard]] Type type() const override { return Type::Directional; }

    glm::vec3 direction{};
};

} // namespace Phos
