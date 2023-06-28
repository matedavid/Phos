#pragma once

#include "core.h"

#include <glm/glm.hpp>

namespace Phos {

// Forward declarations
class Shader;
class Texture;

class Material {
  public:
    struct Definition {
        std::shared_ptr<Shader> shader;
        std::string name;

        std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
    };

    virtual ~Material() = default;

    static std::shared_ptr<Material> create(const Definition& definition);
};

} // namespace Phos
