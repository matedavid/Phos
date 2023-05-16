#include "vulkan_render_pass.h"

#include <utility>

#include "renderer/backend/vulkan/vulkan_device.h"
#include "renderer/backend/vulkan/vulkan_framebuffer.h"
#include "renderer/backend/vulkan/vulkan_command_buffer.h"
#include "renderer/backend/vulkan/vulkan_image.h"

namespace Phos {

VulkanRenderPass::VulkanRenderPass(Description description) : m_description(std::move(description)), m_begin_info({}) {
    for (const auto& attachment : m_description.target_framebuffer->get_attachments()) {
        VkClearValue clear_value;

        if (VulkanImage::is_depth_format(attachment.image->format())) {
            clear_value.depthStencil.depth = attachment.clear_value.r;
        } else {
            const auto& color = attachment.clear_value;
            clear_value.color = {{color.r, color.g, color.b, 1.0f}};
        }

        m_clear_values.push_back(clear_value);
    }

    m_begin_info = VkRenderPassBeginInfo{};
    m_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

    if (!m_description.presentation_target) {
        m_begin_info.renderPass = m_description.target_framebuffer->get_render_pass();
        m_begin_info.framebuffer = m_description.target_framebuffer->handle();
    }

    m_begin_info.renderArea.offset = {0, 0};
    m_begin_info.renderArea.extent = {m_description.target_framebuffer->width(),
                                      m_description.target_framebuffer->height()};
    m_begin_info.clearValueCount = static_cast<uint32_t>(m_clear_values.size());
    m_begin_info.pClearValues = m_clear_values.data();
}

void VulkanRenderPass::begin(const VulkanCommandBuffer& command_buffer) {
    PS_ASSERT(!m_description.presentation_target, "For presentation render passes, use function with framebuffer")
    vkCmdBeginRenderPass(command_buffer.handle(), &m_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRenderPass::begin(const Phos::VulkanCommandBuffer& command_buffer,
                             const std::shared_ptr<VulkanFramebuffer>& framebuffer) {
    m_begin_info.renderPass = framebuffer->get_render_pass();
    m_begin_info.framebuffer = framebuffer->handle();

    vkCmdBeginRenderPass(command_buffer.handle(), &m_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRenderPass::end(const VulkanCommandBuffer& command_buffer) {
    vkCmdEndRenderPass(command_buffer.handle());
}

} // namespace Phos
