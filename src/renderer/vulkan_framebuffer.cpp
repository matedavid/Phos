#include "vulkan_framebuffer.h"

#include "renderer/vulkan_device.h"
#include "renderer/vulkan_render_pass.h"

VulkanFramebuffer::VulkanFramebuffer(std::shared_ptr<VulkanDevice> device,
                                     const std::shared_ptr<VulkanRenderPass>& render_pass,
                                     uint32_t width,
                                     uint32_t height,
                                     const std::vector<VkImageView>& attachments)
      : m_width(width), m_height(height), m_device(std::move(device)) {
    VkFramebufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    create_info.renderPass = render_pass->handle();
    create_info.attachmentCount = (uint32_t)attachments.size();
    create_info.pAttachments = attachments.data();
    create_info.width = m_width;
    create_info.height = m_height;
    create_info.layers = 1; // TODO: Configurable

    VK_CHECK(vkCreateFramebuffer(m_device->handle(), &create_info, nullptr, &m_framebuffer))
}

VulkanFramebuffer::~VulkanFramebuffer() {
    vkDestroyFramebuffer(m_device->handle(), m_framebuffer, nullptr);
}
