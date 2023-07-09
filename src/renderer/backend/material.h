#pragma once

#include "core.h"

#include <glm/glm.hpp>

namespace Phos {

// Forward declarations
class Shader;
class Texture;

class Material {
  public:
    virtual ~Material() = default;

    static std::shared_ptr<Material> create(const std::shared_ptr<Shader>& shader, const std::string& name);

    virtual void set(const std::string& name, glm::vec3 data) = 0;
    virtual void set(const std::string& name, glm::vec4 data) = 0;
    virtual void set(const std::string& name, std::shared_ptr<Texture> texture) = 0;

    virtual bool bake() = 0;
};

} // namespace Phos
