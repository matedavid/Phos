#include "texture.h"

#include "renderer/backend/vulkan/vulkan_texture.h"

namespace Phos {

std::shared_ptr<Texture> Texture::create(const std::string& path) {
    return std::dynamic_pointer_cast<Texture>(std::make_shared<VulkanTexture>(path));
}

std::shared_ptr<Texture> Texture::create(uint32_t width, uint32_t height) {
    return std::dynamic_pointer_cast<Texture>(std::make_shared<VulkanTexture>(width, height));
}

} // namespace Phos
