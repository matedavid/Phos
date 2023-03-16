#pragma once

#include <vulkan/vulkan.h>

class VulkanPhysicalDevice {
  public:
    VulkanPhysicalDevice(VkPhysicalDevice physical_device);
    ~VulkanPhysicalDevice();

    [[nodiscard]] VkPhysicalDeviceFeatures get_features() const { return m_features; }
    [[nodiscard]] VkPhysicalDeviceProperties get_properties() const { return m_properties; }

  private:
    VkPhysicalDevice m_device;

    VkPhysicalDeviceFeatures m_features;
    VkPhysicalDeviceProperties m_properties;
};
