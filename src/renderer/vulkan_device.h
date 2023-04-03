#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>
#include <string_view>

#include "renderer/vulkan_physical_device.h"

// Forward declarations
class VulkanInstance;

class VulkanDevice {
  public:
    //    VulkanDevice(VulkanPhysicalDevice physical_device,
    //                 VkSurfaceKHR surface,
    //                 const std::vector<const char*>& extensions);

    VulkanDevice(const std::unique_ptr<VulkanInstance>& instance,
                 const VulkanPhysicalDevice::Requirements& requirements);
    ~VulkanDevice();

    [[nodiscard]] VkDevice handle() const { return m_device; }
    [[nodiscard]] VulkanPhysicalDevice physical_device() const { return m_physical_device; }

  private:
    VkDevice m_device;
    VulkanPhysicalDevice m_physical_device;

    VkQueue m_graphics_queue;

    [[nodiscard]] VulkanPhysicalDevice select_physical_device(const std::unique_ptr<VulkanInstance>& instance,
                                                              const VulkanPhysicalDevice::Requirements& reqs) const;
};
