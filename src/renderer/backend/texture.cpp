#include "texture.h"

#include "renderer/backend/renderer.h"
#include "renderer/backend/vulkan/vulkan_texture.h"

namespace Phos {

std::shared_ptr<Texture> Texture::create(const std::string& path) {
    switch (Renderer::graphics_api()) {
    case GraphicsAPI::Vulkan:
        return std::dynamic_pointer_cast<Texture>(std::make_shared<VulkanTexture>(path));
    default:
        PS_FAIL("Vulkan is the only supported api")
    }
}

std::shared_ptr<Texture> Texture::create(uint32_t width, uint32_t height) {
    switch (Renderer::graphics_api()) {
    case GraphicsAPI::Vulkan:
        return std::dynamic_pointer_cast<Texture>(std::make_shared<VulkanTexture>(width, height));
    default:
        PS_FAIL("Vulkan is the only supported api")
    }
}

} // namespace Phos