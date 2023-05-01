#include "vulkan_swapchain.h"

#include "core/window.h"
#include "renderer/backend/vulkan/vulkan_instance.h"
#include "renderer/backend/vulkan/vulkan_device.h"
#include "renderer/backend/vulkan/vulkan_render_pass.h"
#include "renderer/backend/vulkan/vulkan_framebuffer.h"
#include "renderer/backend/vulkan/vulkan_context.h"
#include "renderer/backend/vulkan/vulkan_image.h"

namespace Phos {

VulkanSwapchain::VulkanSwapchain(std::shared_ptr<VulkanRenderPass> render_pass)
      : m_render_pass(std::move(render_pass)) {
    m_surface = VulkanContext::instance->get_surface();

    // Create swapchain
    create();
    // Retrieve swapchain images
    retrieve_swapchain_images();
    // Create presentation framebuffers
    create_framebuffers();
}

VulkanSwapchain::~VulkanSwapchain() {
    cleanup();
}

void VulkanSwapchain::acquire_next_image(VkSemaphore semaphore) {
    const auto result = vkAcquireNextImageKHR(
        VulkanContext::device->handle(), m_swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &m_current_image_idx);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate();
        VK_CHECK(vkAcquireNextImageKHR(
            VulkanContext::device->handle(), m_swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &m_current_image_idx))
    } else if (result != VK_SUBOPTIMAL_KHR) {
        VK_CHECK(result)
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

    VK_CHECK(vkCreateSwapchainKHR(VulkanContext::device->handle(), &create_info, nullptr, &m_swapchain))
}

void VulkanSwapchain::cleanup() {
    // Destroy framebuffers
    m_framebuffers.clear();

    // Destroy image views
    for (const auto& image_view : m_image_views) {
        vkDestroyImageView(VulkanContext::device->handle(), image_view, nullptr);
    }

    // Clear images
    m_images.clear();

    // Destroy depth image view
    vkDestroyImageView(VulkanContext::device->handle(), m_depth_image_view, nullptr);

    // Destroy depth image
    m_depth_image.reset();

    // Destroy swapchain
    vkDestroySwapchainKHR(VulkanContext::device->handle(), m_swapchain, nullptr);
}

VulkanSwapchain::SwapchainInformation VulkanSwapchain::get_swapchain_information() const {
    SwapchainInformation info{};

    // Capabilities
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        VulkanContext::device->physical_device().handle(), m_surface, &info.capabilities))

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
        VulkanContext::device->physical_device().handle(), m_surface, &surface_format_count, nullptr))

    PS_ASSERT(surface_format_count > 0, "No surface formats supported")

    std::vector<VkSurfaceFormatKHR> surface_formats(surface_format_count);
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
        VulkanContext::device->physical_device().handle(), m_surface, &surface_format_count, surface_formats.data()))

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
        VulkanContext::device->physical_device().handle(), m_surface, &present_mode_count, nullptr))

    PS_ASSERT(present_mode_count > 0, "No present modes supported")

    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
        VulkanContext::device->physical_device().handle(), m_surface, &present_mode_count, present_modes.data()))

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

    m_images.resize(image_count);
    vkGetSwapchainImagesKHR(VulkanContext::device->handle(), m_swapchain, &image_count, m_images.data());

    // Create image views
    m_image_views.resize(image_count);
    for (uint32_t idx = 0; idx < m_images.size(); ++idx) {
        const auto& image = m_images[idx];

        VkImageViewCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = image;
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = m_swapchain_info.surface_format.format;
        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;

        VK_CHECK(vkCreateImageView(VulkanContext::device->handle(), &create_info, nullptr, &m_image_views[idx]))
    }

    // Create depth image
    m_depth_image = std::make_unique<VulkanImage>(VulkanImage::Description{
        .width = m_swapchain_info.extent.width,
        .height = m_swapchain_info.extent.height,
        .image_type = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_D32_SFLOAT,
        .initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    });

    VkImageViewCreateInfo image_view_create_info{};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.image = m_depth_image->handle();
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = VK_FORMAT_D32_SFLOAT;
    image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(VulkanContext::device->handle(), &image_view_create_info, nullptr, &m_depth_image_view))
}

void VulkanSwapchain::create_framebuffers() {
    for (const auto& view : m_image_views) {
        const std::vector<VkImageView> attachments = {view, m_depth_image_view};
        m_framebuffers.push_back(std::make_unique<VulkanFramebuffer>(
            m_render_pass, m_swapchain_info.extent.width, m_swapchain_info.extent.height, attachments));
    }
}

} // namespace Phos
