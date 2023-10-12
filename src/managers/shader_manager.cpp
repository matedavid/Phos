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

    // PBR Forward Shaders
    const auto pbr_forward = Shader::create(SHADER_PATH("PBR.Forward.Vert.spv"), SHADER_PATH("PBR.Forward.Frag.spv"));

    m_builtin_shaders.insert(std::make_pair("PBR.Forward", pbr_forward));

    // Base Shaders
    const auto skybox = Shader::create(SHADER_PATH("Skybox.Vert.spv"), SHADER_PATH("Skybox.Frag.spv"));
    const auto blending = Shader::create(SHADER_PATH("Blending.Vert.spv"), SHADER_PATH("Blending.Frag.spv"));
    const auto shadow_map = Shader::create(SHADER_PATH("ShadowMap.Vert.spv"), SHADER_PATH("ShadowMap.Frag.spv"));

    m_builtin_shaders.insert(std::make_pair("Skybox", skybox));
    m_builtin_shaders.insert(std::make_pair("Blending", blending));
    m_builtin_shaders.insert(std::make_pair("ShadowMap", shadow_map));

    // Post Processing
    const auto tone_mapping = Shader::create(SHADER_PATH("ToneMapping.Vert.spv"), SHADER_PATH("ToneMapping.Frag.spv"));
    const auto bloom = Shader::create(SHADER_PATH("Bloom.Compute.spv"));

    m_builtin_shaders.insert(std::make_pair("ToneMapping", tone_mapping));
    m_builtin_shaders.insert(std::make_pair("Bloom", bloom));
}

std::shared_ptr<Shader> ShaderManager::get_builtin_shader(const std::string& name) const {
    const auto it = m_builtin_shaders.find(name);
    PS_ASSERT(it != m_builtin_shaders.end(), "No builtin shader named '{}' exists", name)
    return it->second;
}

} // namespace Phos
