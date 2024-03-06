#include "light.h"

namespace Phos {

Light::Light(glm::vec3 pos, glm::vec4 col, float _intensity) : position(pos), color(col), intensity(_intensity) {}

PointLight::PointLight(glm::vec3 pos, glm::vec4 col, float intensity) : Light(pos, col, intensity) {}

DirectionalLight::DirectionalLight(glm::vec3 pos, glm::vec3 dir, glm::vec4 col, float intensity)
      : Light(pos, col, intensity), direction(dir) {}

} // namespace Phos
