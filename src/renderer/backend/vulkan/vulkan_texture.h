#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <memory>

#include "renderer/backend/vulkan/vulkan_image.h"

namespace Phos {

// Forward declarations
class VulkanDevice;

class VulkanTexture {
  public:
    explicit VulkanTexture(const std::string& path);
    explicit VulkanTexture(uint32_t width, uint32_t height);
    ~VulkanTexture();

    [[nodiscard]] std::shared_ptr<VulkanImage> get_image() { return m_image; }
    [[nodiscard]] VkSampler sampler() const { return m_sampler; }

  private:
    std::shared_ptr<VulkanImage> m_image;

    VkSampler m_sampler{};
};

} // namespace Phos
