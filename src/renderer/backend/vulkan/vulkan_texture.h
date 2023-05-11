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
    ~VulkanTexture();

    [[nodiscard]] VkImageView image_view() const { return m_image->view(); }
    [[nodiscard]] VkSampler sampler() const { return m_sampler; }

  private:
    std::shared_ptr<VulkanImage> m_image;

    VkSampler m_sampler{};
};

} // namespace Phos
