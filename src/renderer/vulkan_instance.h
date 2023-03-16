#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>
#include <memory>

#include "renderer/vulkan_physical_device.h"

class VulkanInstance {
  public:
    VulkanInstance(const std::vector<const char*>& extensions);
    ~VulkanInstance();

    VulkanInstance(const VulkanInstance&) = delete;
    VulkanInstance& operator=(const VulkanInstance&) = delete;


  private:
    VkInstance m_instance;
    std::vector<std::unique_ptr<VulkanPhysicalDevice>> m_physical_devices;

    void get_physical_devices();
};
