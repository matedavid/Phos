#include "vulkan_framebuffer.h"

#include "renderer/backend/vulkan/vulkan_device.h"
#include "renderer/backend/vulkan/vulkan_render_pass.h"
#include "renderer/backend/vulkan/vulkan_context.h"

namespace Phos {

VulkanFramebuffer::VulkanFramebuffer(const std::shared_ptr<VulkanRenderPass>& render_pass,
                                     uint32_t width,
                                     uint32_t height,
                                     const std::vector<VkImageView>& attachments)
      : m_width(width), m_height(height) {
    VkFramebufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    create_info.renderPass = render_pass->handle();
    create_info.attachmentCount = (uint32_t)attachments.size();
    create_info.pAttachments = attachments.data();
    create_info.width = m_width;
    create_info.height = m_height;
    create_info.layers = 1; // TODO: Configurable

    VK_CHECK(vkCreateFramebuffer(VulkanContext::device->handle(), &create_info, nullptr, &m_framebuffer))
}

VulkanFramebuffer::~VulkanFramebuffer() {
    vkDestroyFramebuffer(VulkanContext::device->handle(), m_framebuffer, nullptr);
}

} // namespace Phos