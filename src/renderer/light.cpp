#include "light.h"

namespace Phos {

Light::Light(glm::vec3 pos, glm::vec4 col) : position(pos), color(col) {}

PointLight::PointLight(glm::vec3 pos, glm::vec4 col) : Light(pos, col) {}

DirectionalLight::DirectionalLight(glm::vec3 pos, glm::vec3 dir, glm::vec4 col) : Light(pos, col), direction(dir) {}

} // namespace Phos
