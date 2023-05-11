#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <memory>

namespace Phos {

class VulkanImage {
  public:
    enum class Type {
        Image2D,
        Image3D
    };

    enum class Format {
        B8G8R8_SRGB,
        D32_SFLOAT
    };

    struct Description {
        uint32_t width{};
        uint32_t height{};
        Type type = Type::Image2D;
        Format format = Format::B8G8R8_SRGB;

        bool transfer = false; // Will the image be used for transfer operations
    };

    explicit VulkanImage(const Description& description);
    ~VulkanImage();

    void transition_layout(VkImageLayout old_layout, VkImageLayout new_layout) const;

    [[nodiscard]] uint32_t width() const { return m_width; }
    [[nodiscard]] uint32_t height() const { return m_height; }

    [[nodiscard]] VkImage handle() const { return m_image; }
    [[nodiscard]] VkImageView view() const { return m_image_view; }

  private:
    VkImage m_image{VK_NULL_HANDLE};
    VkDeviceMemory m_memory{VK_NULL_HANDLE};

    VkImageView m_image_view{VK_NULL_HANDLE};

    uint32_t m_width, m_height;

    [[nodiscard]] VkImageType get_image_type(Type type) const;
    [[nodiscard]] VkFormat get_image_format(Format format) const;
    [[nodiscard]] VkImageViewType get_image_view_type(Type type) const;

    [[nodiscard]] bool is_depth_format(Format format) const;
};

} // namespace Phos
