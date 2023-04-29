#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <memory>

// Forward declarations
class VulkanDevice;

class VulkanImage {
  public:
    struct Description {
        uint32_t width;
        uint32_t height;
        VkImageType image_type;
        VkFormat format;
        VkImageLayout initial_layout;
        VkImageUsageFlags usage;
    };

    explicit VulkanImage(const Description& description);
    ~VulkanImage();

    void transition_layout(VkImageLayout old_layout, VkImageLayout new_layout) const;

    [[nodiscard]] uint32_t width() const { return m_width; }
    [[nodiscard]] uint32_t height() const { return m_height; }

    [[nodiscard]] VkImage handle() const { return m_image; }

  private:
    VkImage m_image;
    VkDeviceMemory m_memory;

    uint32_t m_width, m_height;
};
