#pragma once

#include <vulkan/vulkan.h>
#include <memory>

#include "utility/logging.h"
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
    [[nodiscard]] uint32_t num_mips() const override { return m_num_mips; }

    [[nodiscard]] VkImage handle() const { return m_image; }
    [[nodiscard]] VkImageView view() const { return m_image_view; }
    [[nodiscard]] VkImageView mip_view(uint32_t mip_level) const {
        PHOS_ASSERT(m_description.generate_mips && mip_level < m_num_mips, "Mip level not valid or mips not requested");
        return m_mip_image_views[mip_level];
    }

    [[nodiscard]] const Description& description() const { return m_description; }

    // @NOTE: Helper methods, maybe move to another place
    [[nodiscard]] static VkImageType get_image_type(Type type);
    [[nodiscard]] static VkFormat get_image_format(Format format);
    [[nodiscard]] static VkImageViewType get_image_view_type(Type type);

    [[nodiscard]] static bool is_depth_format(Format format);

  private:
    VkImage m_image{VK_NULL_HANDLE};
    VkDeviceMemory m_memory{VK_NULL_HANDLE};

    VkImageView m_image_view{VK_NULL_HANDLE};
    std::vector<VkImageView> m_mip_image_views;

    Description m_description;

    bool m_created_resources = true;
    uint32_t m_num_mips = 1;

    void create_image_view(const Description& description);
    [[nodiscard]] uint32_t compute_num_mips() const;
};

} // namespace Phos
