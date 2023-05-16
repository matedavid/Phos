#pragma once

#include "vk_core.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <memory>

namespace Phos {

// Forward declarations
class VulkanDevice;
class VulkanRenderPass;
class VulkanImage;

enum class LoadOperation {
    Load,
    Clear,
    DontCare
};

enum class StoreOperation {
    Store,
    DontCare
};

class VulkanFramebuffer {
  public:
    struct Attachment {
        std::shared_ptr<VulkanImage> image;

        LoadOperation load_operation;
        StoreOperation store_operation;

        glm::vec3 clear_value;

        bool is_presentation = false; // If attachment image will be used for presentation
    };

    struct Description {
        std::vector<Attachment> attachments;
    };

    explicit VulkanFramebuffer(const Description& description);
    explicit VulkanFramebuffer(const Description& description, VkRenderPass render_pass);

    //    VulkanFramebuffer(const std::shared_ptr<VulkanRenderPass>& render_pass,
    //                      uint32_t width,
    //                      uint32_t height,
    //                      const std::vector<VkImageView>& attachments);
    ~VulkanFramebuffer();

    [[nodiscard]] uint32_t width() const { return m_width; }
    [[nodiscard]] uint32_t height() const { return m_height; }

    [[nodiscard]] const std::vector<Attachment>& get_attachments() const { return m_description.attachments; }

    [[nodiscard]] VkFramebuffer handle() const { return m_framebuffer; }
    [[nodiscard]] VkRenderPass get_render_pass() const { return m_render_pass; }

  private:
    // If the framebuffer created the render pass
    bool m_created_render_pass = true;

    VkRenderPass m_render_pass{VK_NULL_HANDLE};
    VkFramebuffer m_framebuffer{VK_NULL_HANDLE};

    std::shared_ptr<VulkanDevice> m_device;

    uint32_t m_width, m_height;

    Description m_description;

    [[nodiscard]] static VkAttachmentLoadOp get_load_op(LoadOperation operation);
    [[nodiscard]] static VkAttachmentStoreOp get_store_op(StoreOperation operation);
};

} // namespace Phos
