#include "shader_manager.h"

#include "renderer/backend/shader.h"

namespace Phos {

#define SHADER_PATH(name) std::string("../shaders/build/") + name

ShaderManager::ShaderManager() {
    // PBR Deferred Shaders
    const auto pbr_geometry_deferred =
        Shader::create(SHADER_PATH("PBR.Geometry.Deferred.Vert.spv"), SHADER_PATH("PBR.Geometry.Deferred.Frag.spv"));
    const auto pbr_lighting_deferred =
        Shader::create(SHADER_PATH("PBR.Lighting.Deferred.Vert.spv"), SHADER_PATH("PBR.Lighting.Deferred.Frag.spv"));

    m_builtin_shaders.insert(std::make_pair("PBR.Geometry.Deferred", pbr_geometry_deferred));
    m_builtin_shaders.insert(std::make_pair("PBR.Lighting.Deferred", pbr_lighting_deferred));
}

std::shared_ptr<Shader> ShaderManager::get_builtin_shader(const std::string& name) const {
    const auto it = m_builtin_shaders.find(name);
    PS_ASSERT(it != m_builtin_shaders.end(), "No builtin shader named '{}' exists", name)
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
