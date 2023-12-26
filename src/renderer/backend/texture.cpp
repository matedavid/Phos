#include "texture.h"

#include "utility/logging.h"

#include "renderer/backend/renderer.h"

#include "renderer/backend/vulkan/vulkan_texture.h"

namespace Phos {

std::shared_ptr<Texture> Texture::create(const std::string& path) {
    switch (Renderer::graphics_api()) {
    case GraphicsAPI::Vulkan:
        return std::dynamic_pointer_cast<Texture>(std::make_shared<VulkanTexture>(path));
    default:
        PHOS_FAIL("Vulkan is the only supported api");
    }
}

std::shared_ptr<Texture> Texture::create(uint32_t width, uint32_t height) {
    switch (Renderer::graphics_api()) {
    case GraphicsAPI::Vulkan:
        return std::dynamic_pointer_cast<Texture>(std::make_shared<VulkanTexture>(width, height));
    default:
        PHOS_FAIL("Vulkan is the only supported api");
    }
}

std::shared_ptr<Texture> Texture::create(const std::vector<char>& data, uint32_t width, uint32_t height) {
    switch (Renderer::graphics_api()) {
    case GraphicsAPI::Vulkan:
        return std::dynamic_pointer_cast<Texture>(std::make_shared<VulkanTexture>(data, width, height));
    default:
        PHOS_FAIL("Vulkan is the only supported api");
    }
}

std::shared_ptr<Texture> Texture::create(const std::shared_ptr<Image>& image) {
    switch (Renderer::graphics_api()) {
    case GraphicsAPI::Vulkan:
        return std::dynamic_pointer_cast<Texture>(std::make_shared<VulkanTexture>(image));
    default:
        PHOS_FAIL("Vulkan is the only supported api");
    }
}

std::shared_ptr<Texture> Texture::white(uint32_t width, uint32_t height) {
    const auto data = std::vector<char>(width * height * 4, (char)255);
    return Texture::create(data, width, height);
}

} // namespace Phos
