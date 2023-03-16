#include "vulkan_instance.h"

#include <iostream>

VulkanInstance::VulkanInstance(const std::vector<const char*>& extensions) {
    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = "vulkan-renderer";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    applicationInfo.pEngineName = "vulkan-renderer";
    applicationInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &applicationInfo;

    // TODO: Enable validation layers
    createInfo.enabledLayerCount = 0;

    createInfo.enabledExtensionCount = (uint32_t)extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        exit(0);
    }

    // Get physical devices
    get_physical_devices();

    for (const auto& device : m_physical_devices) {
        std::cout << "Device name: " << device->get_properties().deviceName << "\n";
    }
}

VulkanInstance::~VulkanInstance() {
    vkDestroyInstance(m_instance, nullptr);
}

void VulkanInstance::get_physical_devices() {
    uint32_t number_devices;
    vkEnumeratePhysicalDevices(m_instance, &number_devices, nullptr);

    std::vector<VkPhysicalDevice> physical_devices(number_devices);
    vkEnumeratePhysicalDevices(m_instance, &number_devices, physical_devices.data());

    for (const auto& device : physical_devices) {
        m_physical_devices.push_back(std::make_unique<VulkanPhysicalDevice>(device));
    }
}
