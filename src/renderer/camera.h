#pragma once

#include <glm/glm.hpp>

namespace Phos {

class Camera {
  public:
    virtual ~Camera() = default;

    void set_position(const glm::vec3& position);
    void rotate(const glm::vec2& rotation);

    [[nodiscard]] glm::vec3 position() const;
    [[nodiscard]] glm::vec3 non_rotated_position() const { return m_position; }

    [[nodiscard]] const glm::mat4& view_matrix() const { return m_view; }
    [[nodiscard]] const glm::mat4& projection_matrix() const { return m_projection; }

  protected:
    glm::mat4 m_projection;
    glm::mat4 m_view;

    glm::vec3 m_position{0.0f, 0.0f, 0.0f};
    glm::vec2 m_rotation{0.0f, 0.0f};

    void recalculate_view_matrix();
};

class PerspectiveCamera : public Camera {
  public:
    explicit PerspectiveCamera();
    explicit PerspectiveCamera(float fov, float aspect, float znear, float zfar);
    ~PerspectiveCamera() override = default;
};

class OrthographicCamera : public Camera {
  public:
    explicit OrthographicCamera();
    explicit OrthographicCamera(float left, float right, float bottom, float top);
    ~OrthographicCamera() override = default;
};

} // namespace Phos
