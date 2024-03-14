#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Phos {

// Forward declarations
struct AABB;

struct Plane {
    glm::vec3 normal = glm::vec3(0.0f);
    float distance = 0.0f;
};

struct Frustum {
    Plane top;
    Plane bottom;
    Plane left;
    Plane right;
    Plane near;
    Plane far;
};

class Camera {
  public:
    enum class Type {
        Perspective,
        Orthographic,
    };

    virtual ~Camera() = default;

    void set_position(const glm::vec3& position);
    void set_rotation(const glm::quat& rotation);
    void rotate(const glm::quat& rotation);

    [[nodiscard]] virtual bool is_inside_frustum(const AABB& aabb) const = 0;

    [[nodiscard]] glm::vec3 position() const;
    [[nodiscard]] glm::vec3 non_rotated_position() const { return m_position; }
    [[nodiscard]] glm::quat rotation() const { return m_rotation; }

    [[nodiscard]] const glm::mat4& view_matrix() const { return m_view; }
    [[nodiscard]] const glm::mat4& projection_matrix() const { return m_projection; }

  protected:
    glm::mat4 m_projection{};
    glm::mat4 m_view{};

    glm::vec3 m_position{0.0f, 0.0f, 0.0f};
    glm::quat m_rotation{1.0f, 0.0f, 0.0f, 0.0f};

    void recalculate_view_matrix();
    virtual void recalculate_frustum() = 0;
};

class PerspectiveCamera : public Camera {
  public:
    PerspectiveCamera();
    PerspectiveCamera(float fov, float aspect, float znear, float zfar);
    ~PerspectiveCamera() override = default;

    void set_aspect_ratio(float aspect);
    void set_fov(float fov);

    [[nodiscard]] bool is_inside_frustum(const AABB& aabb) const override;

    [[nodiscard]] float fov() const { return m_fov; }

  private:
    float m_fov, m_aspect, m_znear, m_zfar;
    Frustum m_frustum;

    void recalculate_projection_matrix();
    void recalculate_frustum() override;
};

/*
class OrthographicCamera : public Camera {
  public:
    OrthographicCamera();
    OrthographicCamera(float left, float right, float bottom, float top);
    OrthographicCamera(float left, float right, float bottom, float top, float znear, float zfar);
    ~OrthographicCamera() override = default;
};
*/

} // namespace Phos
