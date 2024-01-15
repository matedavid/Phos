#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "asset/asset.h"

namespace Phos {

// Forward declarations
class Shader;
class Texture;

class Material : public IAsset {
  public:
    ~Material() override = default;

    [[nodiscard]] AssetType asset_type() override { return AssetType::Material; }

    static std::shared_ptr<Material> create(const std::shared_ptr<Shader>& shader, const std::string& name);

    virtual void set(const std::string& name, float data) = 0;
    virtual void set(const std::string& name, glm::vec3 data) = 0;
    virtual void set(const std::string& name, glm::vec4 data) = 0;
    virtual void set(const std::string& name, std::shared_ptr<Texture> texture) = 0;

    virtual bool bake() = 0;

    [[nodiscard]] virtual const std::string& name() const = 0;
};

} // namespace Phos
