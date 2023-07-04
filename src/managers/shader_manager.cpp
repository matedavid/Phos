#include "shader_manager.h"

#include "renderer/backend/shader.h"

namespace Phos {

#define SHADER_PATH(name) std::string("../assets/shaders/") + name

ShaderManager::ShaderManager() {
    // Lighting Shader
    const auto lighting_shader =
        Shader::create(SHADER_PATH("lighting_vertex.spv"), SHADER_PATH("lighting_fragment.spv"));

    m_builtin_shaders.insert(std::make_pair("Lighting", lighting_shader));
}

std::shared_ptr<Shader> ShaderManager::get_builtin_shader(const std::string& name) const {
    const auto it = m_builtin_shaders.find(name);
    PS_ASSERT(it == m_builtin_shaders.end(), "No builtin shader named '{}' exists", name)
    return it->second;
}

std::unique_ptr<ShaderManager> ShaderManager::m_instance = nullptr;

const std::unique_ptr<ShaderManager>& ShaderManager::instance() {
    if (m_instance == nullptr) {
        m_instance = std::make_unique<ShaderManager>();
    }

    return m_instance;
}

} // namespace Phos
