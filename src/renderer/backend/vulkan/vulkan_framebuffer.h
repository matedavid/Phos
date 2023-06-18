#pragma once

#include "vk_core.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <memory>

#include "renderer/backend/framebuffer.h"

namespace Phos {

// Forward declarations
class VulkanDevice;
class VulkanRenderPass;
class VulkanImage;

class VulkanFramebuffer : public Framebuffer {
  public:
    explicit VulkanFramebuffer(const Description& description);
    explicit VulkanFramebuffer(const Description& description, VkRenderPass render_pass);

    ~VulkanFramebuffer() override;

    [[nodiscard]] uint32_t width() const override { return m_width; }
    [[nodiscard]] uint32_t height() const override { return m_height; }

    [[nodiscard]] const std::vector<Attachment>& get_attachments() const override { return m_description.attachments; }

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
