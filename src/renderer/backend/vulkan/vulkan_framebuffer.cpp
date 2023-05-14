#include "vulkan_framebuffer.h"

#include <ranges>
#include <algorithm>

#include "renderer/backend/vulkan/vulkan_device.h"
#include "renderer/backend/vulkan/vulkan_render_pass.h"
#include "renderer/backend/vulkan/vulkan_context.h"
#include "renderer/backend/vulkan/vulkan_image.h"

namespace Phos {

VulkanFramebuffer::VulkanFramebuffer(const Phos::VulkanFramebuffer::Description& description) {
    // Create Framebuffer render pass
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkAttachmentReference> attachment_references;
    for (const auto& attachment : description.attachments) {
        // Description
        VkAttachmentDescription attachment_description{};
        attachment_description.format = VulkanImage::get_image_format(attachment.image->format());
        attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_description.loadOp = get_load_op(attachment.load_operation);
        attachment_description.storeOp = get_store_op(attachment.store_operation);
        attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        if (attachment.is_presentation) {
            attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        } else if (VulkanImage::is_depth_format(attachment.image->format())) {
            attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        } else {
            attachment_description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }

        attachments.push_back(attachment_description);

        // Reference
        VkAttachmentReference attachment_reference{};
        attachment_reference.attachment = (uint32_t)attachments.size() - 1;

        attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        if (attachment_description.finalLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        attachment_references.push_back(attachment_reference);
    }

    const auto is_color_attachment = [](VkAttachmentReference reference) {
        return reference.layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    };

    const auto is_depth_stencil_attachment = [](VkAttachmentReference reference) {
        return reference.layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    };

    std::vector<VkAttachmentReference> color_attachments;
    std::ranges::copy_if(attachment_references, std::back_inserter(color_attachments), is_color_attachment);

    std::vector<VkAttachmentReference> depth_stencil_attachments;
    std::ranges::copy_if(
        attachment_references, std::back_inserter(depth_stencil_attachments), is_depth_stencil_attachment);

    PS_ASSERT(depth_stencil_attachments.size() <= 1, "Can only have 1 depth / stencil attachment")

    // At the moment, render passes will only have one subpass
    VkSubpassDescription subpass_description{};
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description.colorAttachmentCount = static_cast<uint32_t>(color_attachments.size());
    subpass_description.pColorAttachments = color_attachments.data();
    subpass_description.pDepthStencilAttachment =
        depth_stencil_attachments.size() == 1 ? &depth_stencil_attachments[0] : VK_NULL_HANDLE;

    VkSubpassDependency subpass_dependency{};
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0;

    subpass_dependency.srcStageMask = 0;
    subpass_dependency.dstStageMask = 0;
    if (!color_attachments.empty()) {
        subpass_dependency.srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependency.dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    if (subpass_description.pDepthStencilAttachment != VK_NULL_HANDLE) {
        subpass_dependency.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpass_dependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }

    subpass_dependency.srcAccessMask = 0;
    subpass_dependency.dstAccessMask = 0;
    if (!color_attachments.empty())
        subpass_dependency.dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    if (subpass_description.pDepthStencilAttachment != VK_NULL_HANDLE)
        subpass_dependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    // Create render pass
    VkRenderPassCreateInfo render_pass_create_info{};
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    render_pass_create_info.pAttachments = attachments.data();
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass_description;
    render_pass_create_info.dependencyCount = 1;
    render_pass_create_info.pDependencies = &subpass_dependency;

    VK_CHECK(vkCreateRenderPass(VulkanContext::device->handle(), &render_pass_create_info, nullptr, &m_render_pass))

    // Create framebuffer
    m_width = description.attachments[0].image->width();
    m_width = description.attachments[0].image->height();

    std::vector<VkImageView> framebuffer_attachments;
    for (const auto& attachment : description.attachments) {
        framebuffer_attachments.push_back(attachment.image->view());

        PS_ASSERT(m_width == attachment.image->width() && m_height == attachment.image->height(),
                  "All attachments to framebuffer must have the same width and height")
    }

    VkFramebufferCreateInfo framebuffer_create_info{};
    framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_create_info.renderPass = m_render_pass;
    framebuffer_create_info.attachmentCount = static_cast<uint32_t>(framebuffer_attachments.size());
    framebuffer_create_info.pAttachments = framebuffer_attachments.data();
    framebuffer_create_info.width = m_width;
    framebuffer_create_info.height = m_height;
    framebuffer_create_info.layers = 1;

    VK_CHECK(vkCreateFramebuffer(VulkanContext::device->handle(), &framebuffer_create_info, nullptr, &m_framebuffer))
}

VulkanFramebuffer::VulkanFramebuffer(const std::vector<VkImageView>& attachments,
                                     uint32_t width,
                                     uint32_t height,
                                     VkRenderPass render_pass)
      : m_render_pass(render_pass), m_width(width), m_height(height) {
    VkFramebufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    create_info.renderPass = m_render_pass;
    create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    create_info.pAttachments = attachments.data();
    create_info.width = m_width;
    create_info.height = m_height;
    create_info.layers = 1;

    VK_CHECK(vkCreateFramebuffer(VulkanContext::device->handle(), &create_info, nullptr, &m_framebuffer))

    m_created_swapchain = false;
}

/*
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
 */

VulkanFramebuffer::~VulkanFramebuffer() {
    vkDestroyFramebuffer(VulkanContext::device->handle(), m_framebuffer, nullptr);

    if (m_created_swapchain)
        vkDestroyRenderPass(VulkanContext::device->handle(), m_render_pass, nullptr);
}

VkAttachmentLoadOp VulkanFramebuffer::get_load_op(LoadOperation operation) {
    switch (operation) {
    default:
    case LoadOperation::Clear:
        return VK_ATTACHMENT_LOAD_OP_CLEAR;
    case LoadOperation::Load:
        return VK_ATTACHMENT_LOAD_OP_LOAD;
    case LoadOperation::DontCare:
        return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }
}

VkAttachmentStoreOp VulkanFramebuffer::get_store_op(StoreOperation operation) {
    switch (operation) {
    default:
    case StoreOperation::Store:
        return VK_ATTACHMENT_STORE_OP_STORE;
    case StoreOperation::DontCare:
        return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }
}

} // namespace Phos
