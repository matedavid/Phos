#include "vulkan_swapchain.h"

#include "vk_core.h"

#include "core/window.h"
#include "renderer/backend/vulkan/vulkan_instance.h"
#include "renderer/backend/vulkan/vulkan_device.h"
#include "renderer/backend/vulkan/vulkan_framebuffer.h"
#include "renderer/backend/vulkan/vulkan_context.h"
#include "renderer/backend/vulkan/vulkan_image.h"

namespace Phos {

VulkanSwapchain::VulkanSwapchain() {
    m_surface = VulkanContext::instance->get_surface();

    // Create swapchain
    create();
    // Retrieve swapchain images
    retrieve_swapchain_images();
    // Create render pass
    create_render_pass();
    // Create framebuffers
    create_framebuffers();
}

VulkanSwapchain::~VulkanSwapchain() {
    cleanup();

    // Destroy render pass
    vkDestroyRenderPass(VulkanContext::device->handle(), m_render_pass, nullptr);
}

void VulkanSwapchain::acquire_next_image(VkSemaphore semaphore, VkFence fence) {
    const auto result = vkAcquireNextImageKHR(
        VulkanContext::device->handle(), m_swapchain, UINT64_MAX, semaphore, fence, &m_current_image_idx);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate();
        VK_CHECK(vkAcquireNextImageKHR(
            VulkanContext::device->handle(), m_swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &m_current_image_idx));
    } else if (result != VK_SUBOPTIMAL_KHR) {
        VK_CHECK(result);
    }
}

void VulkanSwapchain::recreate() {
    vkDeviceWaitIdle(VulkanContext::device->handle());
    cleanup();

    // Create swapchain
    create();
    // Retrieve swapchain images
    retrieve_swapchain_images();
    // Create presentation framebuffers
    create_framebuffers();
}

void VulkanSwapchain::create() {
    m_swapchain_info = get_swapchain_information();

    VulkanPhysicalDevice::QueueFamilies queue_families =
        VulkanContext::device->physical_device().get_queue_families(VulkanPhysicalDevice::Requirements{
            .graphics = true,
            .presentation = true,
            .surface = m_surface,
        });

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = VulkanContext::instance->get_surface();
    create_info.minImageCount = m_swapchain_info.capabilities.minImageCount + 1;
    create_info.imageFormat = m_swapchain_info.surface_format.format;
    create_info.imageColorSpace = m_swapchain_info.surface_format.colorSpace;
    create_info.imageExtent = m_swapchain_info.extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (queue_families.graphics == queue_families.presentation) {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;
    } else {
        const std::array<uint32_t, 2> queues_indices = {queue_families.graphics, queue_families.presentation};

        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = queues_indices.size();
        create_info.pQueueFamilyIndices = queues_indices.data();
    }

    create_info.preTransform = m_swapchain_info.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = m_swapchain_info.present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    VK_CHECK(vkCreateSwapchainKHR(VulkanContext::device->handle(), &create_info, nullptr, &m_swapchain));
}

void VulkanSwapchain::cleanup() {
    // Destroy framebuffers
    m_framebuffers.clear();

    // Clear images
    m_images.clear();

    // Destroy depth image
    m_depth_image.reset();

    // Destroy swapchain
    vkDestroySwapchainKHR(VulkanContext::device->handle(), m_swapchain, nullptr);
}

VulkanSwapchain::SwapchainInformation VulkanSwapchain::get_swapchain_information() const {
    SwapchainInformation info{};

    // Capabilities
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        VulkanContext::device->physical_device().handle(), m_surface, &info.capabilities));

    // Extent
    if (info.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        info.extent = info.capabilities.currentExtent;
    } else {
        const auto width = VulkanContext::window->get_width();
        const auto height = VulkanContext::window->get_height();

        VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        actualExtent.width = std::clamp(
            actualExtent.width, info.capabilities.minImageExtent.width, info.capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(
            actualExtent.height, info.capabilities.minImageExtent.height, info.capabilities.maxImageExtent.height);

        info.extent.width = actualExtent.width;
        info.extent.height = actualExtent.height;
    }

    // Surface formats
    uint32_t surface_format_count;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
        VulkanContext::device->physical_device().handle(), m_surface, &surface_format_count, nullptr));

    PHOS_ASSERT(surface_format_count > 0, "No surface formats supported");

    std::vector<VkSurfaceFormatKHR> surface_formats(surface_format_count);
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
        VulkanContext::device->physical_device().handle(), m_surface, &surface_format_count, surface_formats.data()));

    // TODO: Make surface format selection configurable?
    info.surface_format = surface_formats[0];

    for (const auto& format : surface_formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            info.surface_format = format;
            break;
        }
    }

    // Present mode
    uint32_t present_mode_count;
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
        VulkanContext::device->physical_device().handle(), m_surface, &present_mode_count, nullptr));

    PHOS_ASSERT(present_mode_count > 0, "No present modes supported");

    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
        VulkanContext::device->physical_device().handle(), m_surface, &present_mode_count, present_modes.data()));

    // TODO: Make present mode selection configurable?
    info.present_mode = present_modes[0];

    for (const auto& present_mode : present_modes) {
        if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            info.present_mode = present_mode;
            break;
        }
    }

    return info;
}

void VulkanSwapchain::retrieve_swapchain_images() {
    // Retrieve images
    uint32_t image_count;
    vkGetSwapchainImagesKHR(VulkanContext::device->handle(), m_swapchain, &image_count, nullptr);

    std::vector<VkImage> images(image_count);
    vkGetSwapchainImagesKHR(VulkanContext::device->handle(), m_swapchain, &image_count, images.data());

    // Create image views
    m_images.clear();
    for (const auto& image : images) {
        const auto description = VulkanImage::Description{
            .width = m_swapchain_info.extent.width,
            .height = m_swapchain_info.extent.height,
            .type = VulkanImage::Type::Image2D,
            // TODO: Hardcoded at the moment, but should be: m_swapchain_info.surface_format.format
            .format = VulkanImage::Format::B8G8R8A8_SRGB,
        };
        m_images.push_back(std::make_shared<VulkanImage>(description, image));
    }

    // Create depth image
    m_depth_image = std::make_shared<VulkanImage>(VulkanImage::Description{
        .width = m_swapchain_info.extent.width,
        .height = m_swapchain_info.extent.height,
        .type = VulkanImage::Type::Image2D,
        .format = VulkanImage::Format::D32_SFLOAT,
        .attachment = true,
    });
}

void VulkanSwapchain::create_framebuffers() {
    PHOS_ASSERT(m_framebuffers.empty(), "Cannot create new framebuffers if array is not empty");

    for (const auto& image : m_images) {
        const auto color_attachment = VulkanFramebuffer::Attachment{
            .image = image,
            .load_operation = LoadOperation::Clear,
            .store_operation = StoreOperation::Store,
            .clear_value = glm::vec3(0.2f, 0.2f, 0.3f),
            .is_presentation = true,
        };

        const auto depth_attachment = VulkanFramebuffer::Attachment{
            .image = m_depth_image,
            .load_operation = LoadOperation::Clear,
            .store_operation = StoreOperation::DontCare,
            .clear_value = glm::vec3(1.0f),
        };

        const auto description = VulkanFramebuffer::Description{
            .attachments = {color_attachment, depth_attachment},
        };
        m_framebuffers.push_back(std::make_shared<VulkanFramebuffer>(description, m_render_pass));
    }
}

void VulkanSwapchain::create_render_pass() {
    VkAttachmentDescription color_attachment_description{};
    color_attachment_description.format = m_swapchain_info.surface_format.format;
    color_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depth_attachment_description{};
    depth_attachment_description.format = VK_FORMAT_D32_SFLOAT;
    depth_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass_description{};
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description.colorAttachmentCount = 1;
    subpass_description.pColorAttachments = &color_attachment_ref;
    subpass_description.pDepthStencilAttachment = &depth_attachment_ref;

    VkSubpassDependency subpass_dependency{};
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.srcAccessMask = 0;
    subpass_dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::vector<VkAttachmentDescription> attachments = {color_attachment_description, depth_attachment_description};

    VkRenderPassCreateInfo render_pass_create_info{};
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    render_pass_create_info.pAttachments = attachments.data();
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass_description;
    render_pass_create_info.dependencyCount = 1;
    render_pass_create_info.pDependencies = &subpass_dependency;

    VK_CHECK(vkCreateRenderPass(VulkanContext::device->handle(), &render_pass_create_info, nullptr, &m_render_pass));
}

} // namespace Phos
