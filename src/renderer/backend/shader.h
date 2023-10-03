#pragma once

#include "core.h"

namespace Phos {

struct ShaderProperty {
    enum class Type {
        Float,
        Vec3,
        Vec4,
        Texture,
    };

    Type type;
    std::string name;
};

class Shader {
  public:
    virtual ~Shader() = default;

    static std::shared_ptr<Shader> create(const std::string& vertex_path, const std::string& fragment_path);
    static std::shared_ptr<Shader> create(const std::string& path);

    [[nodiscard]] virtual std::vector<ShaderProperty> get_shader_properties() const = 0;
};

} // namespace Phos
