#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>

#include "renderer/mesh.h"

namespace Phos {

void Camera::set_position(const glm::vec3& position) {
    m_position = position;
    recalculate_view_matrix();
}

void Camera::set_rotation(const glm::quat& rotation) {
    m_rotation = rotation;
    recalculate_view_matrix();
}

void Camera::rotate(const glm::quat& rotation) {
    m_rotation = rotation * m_rotation;
    recalculate_view_matrix();
}

glm::vec3 Camera::position() const {
    return glm::inverse(m_view)[3];
}

void Camera::recalculate_view_matrix() {
    const auto inverse_view_matrix = glm::translate(glm::mat4(1.0f), m_position) * glm::mat4_cast(m_rotation);
    m_view = glm::inverse(inverse_view_matrix);

    recalculate_frustum();
}

//
// PerspectiveCamera
//

PerspectiveCamera::PerspectiveCamera() : PerspectiveCamera(glm::radians(60.0f), 1.0f, 0.001f, 100.0f) {}

PerspectiveCamera::PerspectiveCamera(float fov, float aspect, float znear, float zfar)
      : m_fov(fov), m_aspect(aspect), m_znear(znear), m_zfar(zfar) {
    recalculate_projection_matrix();
    recalculate_view_matrix();
    set_aspect_ratio(aspect);
}

void PerspectiveCamera::set_aspect_ratio(float aspect) {
    m_aspect = aspect;

    const float tmp_fov = m_fov;
    if (aspect < 1.0f) {
        m_fov = 2.0f * glm::atan(glm::tan(m_fov * 0.5f) / aspect);
    }
    recalculate_projection_matrix();
    m_fov = tmp_fov;
}

void PerspectiveCamera::set_fov(float fov) {
    m_fov = fov;
    recalculate_projection_matrix();
}

bool PerspectiveCamera::is_inside_frustum(const AABB& aabb) const {
    const auto test_plane = [](const AABB& _aabb, const Plane& plane) -> bool {
        const auto center = (_aabb.min + _aabb.max) * 0.5f;
        const auto radius = glm::abs(_aabb.max - _aabb.min) * 0.5f;

        const auto distance = glm::dot(plane.normal, center) + plane.distance;

        return distance >= -glm::dot(glm::abs(plane.normal), radius);
    };

    // clang-format off
    return test_plane(aabb, m_frustum.left)
        && test_plane(aabb, m_frustum.right)
        && test_plane(aabb, m_frustum.bottom)
        && test_plane(aabb, m_frustum.top)
        && test_plane(aabb, m_frustum.near)
        && test_plane(aabb, m_frustum.far);
    // clang-format on
}

void PerspectiveCamera::recalculate_projection_matrix() {
    m_projection = glm::perspective(m_fov, m_aspect, m_znear, m_zfar);

    recalculate_frustum();
}

void PerspectiveCamera::recalculate_frustum() {
    const auto VP = m_projection * m_view;

    // Frustum creation from: https://gdbooks.gitbooks.io/legacyopengl/content/Chapter8/frustum.html
    const auto row1 = glm::row(VP, 0);
    const auto row2 = glm::row(VP, 1);
    const auto row3 = glm::row(VP, 2);
    const auto row4 = glm::row(VP, 3);

    m_frustum.left = {
        .normal = row4 + row1,
        .distance = row4.w + row1.w,
    };

    m_frustum.right = {
        .normal = row4 - row1,
        .distance = row4.w - row1.w,
    };

    m_frustum.bottom = {
        .normal = row4 + row2,
        .distance = row4.w + row2.w,
    };

    m_frustum.top = {
        .normal = row4 - row2,
        .distance = row4.w - row2.w,
    };

    m_frustum.near = {
        .normal = row4 + row3,
        .distance = row4.w + row3.w,
    };

    m_frustum.far = {
        .normal = row4 - row3,
        .distance = row4.w - row3.w,
    };
}

//
// OrthographicCamera
//

/*
OrthographicCamera::OrthographicCamera() {
    m_projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f);
    recalculate_view_matrix();
}

OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
      : OrthographicCamera(left, right, bottom, top, 0.01f, 100.0f) {}

OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top, float znear, float zfar) {
    m_projection = glm::ortho(left, right, bottom, top, znear, zfar);
    recalculate_view_matrix();
}
*/

} // namespace Phos
