#include "vulkan_swapchain.h"

#include "renderer/vulkan_device.h"

VulkanSwapchain::VulkanSwapchain(std::shared_ptr<VulkanDevice>& device, VkSurfaceKHR surface)
    : m_device(std::move(device)), m_surface(surface) {
    SwapchainInformation info = get_swapchain_information();

    VulkanPhysicalDevice::QueueFamilies queue_families =
        m_device->physical_device().get_queue_families(VulkanPhysicalDevice::Requirements{
            .graphics = true,
            .presentation = true,
            .surface = surface,
        });

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface;
    create_info.minImageCount = info.capabilities.minImageCount + 1;
    create_info.imageFormat = info.surface_format.format;
    create_info.imageColorSpace = info.surface_format.colorSpace;
    create_info.imageExtent = info.extent;
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

    create_info.preTransform = info.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = info.present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    VK_CHECK(vkCreateSwapchainKHR(m_device->handle(), &create_info, nullptr, &m_swapchain))
}

VulkanSwapchain::~VulkanSwapchain() {
    vkDestroySwapchainKHR(m_device->handle(), m_swapchain, nullptr);
}

VulkanSwapchain::SwapchainInformation VulkanSwapchain::get_swapchain_information() const {
    SwapchainInformation info{};

    // Capabilities
    VK_CHECK(
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->physical_device().handle(), m_surface, &info.capabilities))

    // Extent
    if (info.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        info.extent = info.capabilities.currentExtent;
    } else {
        /*
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        actualExtent.width = std::clamp(actualExtent.width, surfaceCapabilities.minImageExtent.width,
                                        surfaceCapabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, surfaceCapabilities.minImageExtent.height,
                                         surfaceCapabilities.maxImageExtent.height);

        information.extent.width = (uint32_t)width;
        information.extent.height = (uint32_t)height;
         */
        CORE_FAIL("Not implemented");
    }

    // Surface formats
    uint32_t surface_format_count;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
        m_device->physical_device().handle(), m_surface, &surface_format_count, nullptr))

    CORE_ASSERT(surface_format_count > 0, "No surface formats supported")

    std::vector<VkSurfaceFormatKHR> surface_formats(surface_format_count);
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
        m_device->physical_device().handle(), m_surface, &surface_format_count, surface_formats.data()))

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
        m_device->physical_device().handle(), m_surface, &present_mode_count, nullptr))

    CORE_ASSERT(present_mode_count > 0, "No present modes supported")

    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
        m_device->physical_device().handle(), m_surface, &present_mode_count, present_modes.data()))

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
