#pragma once

#include "core.h"

namespace Phos {

// Forward declarations
class Shader;

class ShaderManager {
  public:
    ShaderManager();
    ~ShaderManager() = default;

    [[nodiscard]] std::shared_ptr<Shader> get_builtin_shader(const std::string& name) const;

    [[nodiscard]] static const std::unique_ptr<ShaderManager>& instance();

  private:
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_builtin_shaders;

    static std::unique_ptr<ShaderManager> m_instance;
};

} // namespace Phos
