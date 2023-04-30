#include "vulkan_render_pass.h"

#include "renderer/backend/vulkan/vulkan_device.h"
#include "renderer/backend/vulkan/vulkan_framebuffer.h"
#include "renderer/backend/vulkan/vulkan_command_buffer.h"
#include "renderer/backend/vulkan/vulkan_context.h"

namespace Phos {

VulkanRenderPass::VulkanRenderPass() {
    // TODO: This should definitely be configurable

    VkAttachmentDescription attachment_description{};
    attachment_description.format = VK_FORMAT_B8G8R8A8_SRGB;
    attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment{};
    color_attachment.attachment = 0;
    color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass_description{};
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description.colorAttachmentCount = 1;
    subpass_description.pColorAttachments = &color_attachment;

    VkSubpassDependency subpass_dependency{};
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.srcAccessMask = 0;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_create_info{};
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount = 1;
    render_pass_create_info.pAttachments = &attachment_description;
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass_description;
    render_pass_create_info.dependencyCount = 1;
    render_pass_create_info.pDependencies = &subpass_dependency;

    VK_CHECK(vkCreateRenderPass(VulkanContext::device->handle(), &render_pass_create_info, nullptr, &m_render_pass))
}

VulkanRenderPass::~VulkanRenderPass() {
    vkDestroyRenderPass(VulkanContext::device->handle(), m_render_pass, nullptr);
}

void VulkanRenderPass::begin(const VulkanCommandBuffer& command_buffer, const VulkanFramebuffer& framebuffer) {
    VkClearValue clear{};
    clear.color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassBeginInfo info{};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.renderPass = m_render_pass;
    info.framebuffer = framebuffer.handle();
    info.renderArea.offset = {0, 0};
    info.renderArea.extent = {framebuffer.width(), framebuffer.height()};
    info.clearValueCount = 1;
    info.pClearValues = &clear;

    vkCmdBeginRenderPass(command_buffer.handle(), &info, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRenderPass::end(const VulkanCommandBuffer& command_buffer) {
    vkCmdEndRenderPass(command_buffer.handle());
}

} // namespace Phos