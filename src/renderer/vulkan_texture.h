#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <memory>

// Forward declarations
class VulkanDevice;
class VulkanImage;

class VulkanTexture {
  public:
    explicit VulkanTexture(const std::string& path);
    ~VulkanTexture();

    [[nodiscard]] VkImageView image_view() const { return m_image_view; }
    [[nodiscard]] VkSampler sampler() const { return m_sampler; }

  private:
    std::unique_ptr<VulkanImage> m_image;

    VkImageView m_image_view{};
    VkSampler m_sampler{};
};
