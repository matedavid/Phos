#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

#include "renderer/vulkan_physical_device.h"

class VulkanDevice {
  public:
    VulkanDevice(VulkanPhysicalDevice physical_device,
                 VkSurfaceKHR surface,
                 const std::vector<const char*>& extensions);
    ~VulkanDevice();

    [[nodiscard]] VkDevice handle() const { return m_device; }
    [[nodiscard]] VulkanPhysicalDevice physical_device() const { return m_physical_device; }

  private:
    VkDevice m_device;
    VulkanPhysicalDevice m_physical_device;

    VkQueue m_graphics_queue;
};
