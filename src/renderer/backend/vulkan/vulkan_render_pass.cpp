#include "vulkan_render_pass.h"

#include <utility>

#include "renderer/backend/vulkan/vulkan_device.h"
#include "renderer/backend/vulkan/vulkan_framebuffer.h"
#include "renderer/backend/vulkan/vulkan_command_buffer.h"
#include "renderer/backend/vulkan/vulkan_image.h"

namespace Phos {

static std::vector<VkClearValue> get_clear_values(const std::shared_ptr<VulkanFramebuffer>& framebuffer) {
    std::vector<VkClearValue> clear_values;

    for (const auto& attachment : framebuffer->get_attachments()) {
        VkClearValue clear_value;

        if (VulkanImage::is_depth_format(attachment.image->format())) {
            clear_value.depthStencil.depth = attachment.clear_value.r;
        } else {
            const auto& color = attachment.clear_value;
            clear_value.color = {{color.r, color.g, color.b, 1.0f}};
        }

        clear_values.push_back(clear_value);
    }

    return clear_values;
}

VulkanRenderPass::VulkanRenderPass(Description description) : m_description(std::move(description)), m_begin_info({}) {
    PS_ASSERT(m_description.presentation_target || m_description.target_framebuffer != nullptr,
              "You must provide a target framebuffer");

    // TODO: Maybe do differently?
    const auto target_framebuffer = m_description.target_framebuffer != nullptr
                                        ? std::dynamic_pointer_cast<VulkanFramebuffer>(m_description.target_framebuffer)
                                        : nullptr;

    if (target_framebuffer != nullptr) {
        m_clear_values = get_clear_values(target_framebuffer);
    }

    m_begin_info = VkRenderPassBeginInfo{};
    m_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

    if (!m_description.presentation_target) {
        m_begin_info.renderPass = target_framebuffer->get_render_pass();
        m_begin_info.framebuffer = target_framebuffer->handle();

        m_begin_info.renderArea.offset = {0, 0};
        m_begin_info.renderArea.extent = {
            m_description.target_framebuffer->width(),
            m_description.target_framebuffer->height(),
        };
    }

    m_begin_info.clearValueCount = static_cast<uint32_t>(m_clear_values.size());
    m_begin_info.pClearValues = m_clear_values.data();
}

void VulkanRenderPass::begin(const std::shared_ptr<CommandBuffer>& command_buffer) {
    // TODO: Maybe do differently?
    const auto native_command_buffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(command_buffer);

    PS_ASSERT(!m_description.presentation_target, "For presentation render passes, use function with framebuffer")
    vkCmdBeginRenderPass(native_command_buffer->handle(), &m_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRenderPass::begin(const std::shared_ptr<CommandBuffer>& command_buffer,
                             const std::shared_ptr<Framebuffer>& framebuffer) {
    // TODO: Maybe do differently?
    const auto& native_command_buffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(command_buffer);
    const auto& native_framebuffer = std::dynamic_pointer_cast<VulkanFramebuffer>(framebuffer);

    m_begin_info.renderPass = native_framebuffer->get_render_pass();
    m_begin_info.framebuffer = native_framebuffer->handle();

    const auto clear_values = get_clear_values(native_framebuffer);
    m_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
    m_begin_info.pClearValues = clear_values.data();

    m_begin_info.renderArea.offset = {0, 0};
    m_begin_info.renderArea.extent = {
        framebuffer->width(),
        framebuffer->height(),
    };

    vkCmdBeginRenderPass(native_command_buffer->handle(), &m_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRenderPass::end(const std::shared_ptr<CommandBuffer>& command_buffer) {
    // TODO: Maybe do differently?
    const auto& native_command_buffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(command_buffer);

    vkCmdEndRenderPass(native_command_buffer->handle());
}

} // namespace Phos
