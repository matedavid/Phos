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
        bool attachment = false; // Will the image be used as an attachment of a Framebuffer
    };

    explicit VulkanImage(const Description& description);
    explicit VulkanImage(const Description& description, VkImage image);
    ~VulkanImage();

    void transition_layout(VkImageLayout old_layout, VkImageLayout new_layout) const;

    [[nodiscard]] uint32_t width() const { return m_width; }
    [[nodiscard]] uint32_t height() const { return m_height; }
    [[nodiscard]] Format format() const { return m_format; }

    [[nodiscard]] VkImage handle() const { return m_image; }
    [[nodiscard]] VkImageView view() const { return m_image_view; }

    // Helper methods, maybe move to another place
    [[nodiscard]] static VkImageType get_image_type(Type type);
    [[nodiscard]] static VkFormat get_image_format(Format format);
    [[nodiscard]] static VkImageViewType get_image_view_type(Type type);

    [[nodiscard]] static bool is_depth_format(Format format);

  private:
    VkImage m_image{VK_NULL_HANDLE};
    VkDeviceMemory m_memory{VK_NULL_HANDLE};

    VkImageView m_image_view{VK_NULL_HANDLE};

    uint32_t m_width, m_height;
    Format m_format;

    bool m_created_resources = true;

    void create_image_view(const Description& description);
};

} // namespace Phos
