#include "vulkan_device.h"

#include "renderer/vulkan_physical_device.h"

VulkanDevice::VulkanDevice(
    VulkanPhysicalDevice physical_device,
    VkSurfaceKHR surface,
    const std::vector<const char*>& extensions)
    : m_physical_device(physical_device) {
    // TODO: Should make queue families configurable?
    const auto queue_families = physical_device.get_queue_families({
        .graphics = true,
        .presentation = true,
        .surface = surface,
    });

    float priority = 1.0f;

    VkDeviceQueueCreateInfo graphics_queue_create_info{};
    graphics_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    graphics_queue_create_info.queueFamilyIndex = queue_families.graphics;
    graphics_queue_create_info.queueCount = 1;
    graphics_queue_create_info.pQueuePriorities = &priority;

    const std::array<VkDeviceQueueCreateInfo, 1> queues = {graphics_queue_create_info};

    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = queues.size();
    create_info.pQueueCreateInfos = queues.data();
    create_info.enabledExtensionCount = (uint32_t)extensions.size();
    create_info.ppEnabledExtensionNames = extensions.data();

    VK_CHECK(vkCreateDevice(m_physical_device.handle(), &create_info, nullptr, &m_device))

    vkGetDeviceQueue(m_device, queue_families.graphics, 0, &m_graphics_queue);
}

VulkanDevice::~VulkanDevice() {
    vkDestroyDevice(m_device, nullptr);
}
