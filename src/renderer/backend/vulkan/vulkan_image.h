#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <memory>

#include "renderer/backend/image.h"

namespace Phos {

class VulkanImage : public Image {
  public:
    explicit VulkanImage(const Description& description);
    explicit VulkanImage(const Description& description, VkImage image);
    ~VulkanImage() override;

    void transition_layout(VkImageLayout old_layout, VkImageLayout new_layout) const;

    [[nodiscard]] uint32_t width() const override { return m_description.width; }
    [[nodiscard]] uint32_t height() const override { return m_description.height; }
    [[nodiscard]] Format format() const override { return m_description.format; }
    [[nodiscard]] uint32_t num_layers() const { return m_description.num_layers; }

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

    Description m_description;

    bool m_created_resources = true;

    void create_image_view(const Description& description);
};

} // namespace Phos
