#include "vulkan_context.h"

#include <GLFW/glfw3.h>

#include <iostream>
#include <ranges>
#include <optional>

VulkanContext::VulkanContext(const std::vector<const char*>& required_extensions, GLFWwindow* window) {
    m_instance = std::make_unique<VulkanInstance>(required_extensions);

    const auto physical_devices = m_instance->get_physical_devices();

    // Create surface
    VK_CHECK(glfwCreateWindowSurface(m_instance->handle(), window, nullptr, &m_surface))

    const std::vector<const char*> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    const auto selected_physical_device = select_physical_device(physical_devices, device_extensions);

    std::cout << "Selected physical device: " << selected_physical_device.get_properties().deviceName << "\n";

    const auto device = std::make_shared<VulkanDevice>(selected_physical_device, m_surface, device_extensions);
}

VulkanContext::~VulkanContext() {
    vkDestroySurfaceKHR(m_instance->handle(), m_surface, nullptr);
}

VulkanPhysicalDevice VulkanContext::select_physical_device(
    const std::vector<VulkanPhysicalDevice>& physical_devices,
    const std::vector<const char*>& extensions) const {
    const VulkanPhysicalDevice::Requirements requirements = {
        .graphics = true,
        .transfer = true,
        .presentation = true,
        .surface = m_surface,

        .extensions = extensions,
    };

    const auto is_device_suitable = [&requirements](const VulkanPhysicalDevice& device) -> bool {
        return device.is_suitable(requirements);
    };

    // TODO: Should make these parameters configurable
    constexpr bool graphics_transfer_same_queue = true;
    constexpr bool graphics_presentation_same_queue = false;

    uint32_t max_score = 0;
    std::optional<VulkanPhysicalDevice> max_device;

    for (const auto& device : physical_devices | std::views::filter(is_device_suitable)) {
        uint32_t score = 0;

        const auto queue_families = device.get_queue_families(requirements);

        if (graphics_transfer_same_queue && queue_families.graphics == queue_families.transfer)
            score += 10;

        if (graphics_presentation_same_queue && queue_families.graphics == queue_families.presentation)
            score += 10;

        if (score > max_score) {
            max_device = device;
            max_score = score;
        }
    }

    CORE_ASSERT(max_device.has_value(), "There are no suitable devices")

    return max_device.value();
}
