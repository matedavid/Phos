#include "vulkan_physical_device.h"

VulkanPhysicalDevice::VulkanPhysicalDevice(VkPhysicalDevice physical_device) : m_device(physical_device) {
    vkGetPhysicalDeviceFeatures(m_device, &m_features);
    vkGetPhysicalDeviceProperties(m_device, &m_properties);
}

VulkanPhysicalDevice::~VulkanPhysicalDevice() {
}
