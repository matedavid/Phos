#include "shader.h"

#include "renderer/backend/vulkan/vulkan_shader.h"

namespace Phos {

std::shared_ptr<Shader> Shader::create(const std::string& vertex_path, const std::string& fragment_path) {
    return std::dynamic_pointer_cast<Shader>(std::make_shared<VulkanShader>(vertex_path, fragment_path));
}

}
