#include "shader.h"

#include "utility/logging.h"

#include "renderer/backend/renderer.h"

#include "renderer/backend/vulkan/vulkan_shader.h"

namespace Phos {

std::shared_ptr<Shader> Shader::create(const std::string& vertex_path, const std::string& fragment_path) {
    switch (Renderer::graphics_api()) {
    case GraphicsAPI::Vulkan:
        return std::dynamic_pointer_cast<Shader>(std::make_shared<VulkanShader>(vertex_path, fragment_path));
    default:
        PHOS_FAIL("Vulkan is the only supported api");
    }
}

std::shared_ptr<Shader> Shader::create(const std::string& path) {
    switch (Renderer::graphics_api()) {
    case GraphicsAPI::Vulkan:
        return std::dynamic_pointer_cast<Shader>(std::make_shared<VulkanShader>(path));
    default:
        PHOS_FAIL("Vulkan is the only supported api");
    }
}

} // namespace Phos
