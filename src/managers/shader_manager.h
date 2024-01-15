#pragma once

#include <memory>
#include <unordered_map>

namespace Phos {

// Forward declarations
class Shader;

class ShaderManager {
  public:
    ShaderManager();
    ~ShaderManager() = default;

    [[nodiscard]] std::shared_ptr<Shader> get_builtin_shader(const std::string& name) const;

  private:
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_builtin_shaders;
};

} // namespace Phos
