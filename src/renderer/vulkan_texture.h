#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <memory>

// Forward declarations
class VulkanDevice;

class VulkanTexture {
  public:
    VulkanTexture(std::shared_ptr<VulkanDevice> device, const std::string& path);
    ~VulkanTexture();

    [[nodiscard]] VkImageView image_view() const { return m_image_view; }
    [[nodiscard]] VkSampler sampler() const { return m_sampler; }

  private:
    VkImage m_image;
    VkDeviceMemory m_image_memory;

    VkImageView m_image_view;
    VkSampler m_sampler;

    uint32_t m_width, m_height;

    std::shared_ptr<VulkanDevice> m_device;

    void transition_image_layout(VkImage image, VkImageLayout old_layout, VkImageLayout new_layout);
    void copy_buffer_to_image(VkBuffer buffer);
};
