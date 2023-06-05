#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Phos {

void Camera::set_position(const glm::vec3& position) {
    m_position = position;
    recalculate_view_matrix();
}

glm::vec3 Camera::position() const {
    return glm::inverse(m_view)[3];
}

void Camera::recalculate_view_matrix() {
    m_view = glm::mat4(1.0f);
}

//
// PerspectiveCamera
//

PerspectiveCamera::PerspectiveCamera() {
    m_projection = glm::perspective(60.0f, 1.0f, 0.001f, 100.0f);
    recalculate_view_matrix();
}

PerspectiveCamera::PerspectiveCamera(float fov, float aspect, float znear, float zfar) {
    m_projection = glm::perspective(fov, aspect, znear, zfar);
    recalculate_view_matrix();
}

//
// OrthographicCamera
//

OrthographicCamera::OrthographicCamera() {
    m_projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f);
    recalculate_view_matrix();
}

OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top) {
    m_projection = glm::ortho(left, right, bottom, top);
    recalculate_view_matrix();
}

} // namespace Phos
