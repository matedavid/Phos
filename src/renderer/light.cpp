#include "light.h"

namespace Phos {

PointLight::PointLight(glm::vec3 pos, glm::vec4 col) : position(pos), color(col) {}

DirectionalLight::DirectionalLight(glm::vec3 dir, glm::vec4 col) : direction(dir), color(col) {}

} // namespace Phos
