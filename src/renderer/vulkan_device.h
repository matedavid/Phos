#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>
#include <string_view>

#include "renderer/vulkan_physical_device.h"

// Forward declarations
class VulkanInstance;
class VulkanQueue;

class VulkanDevice {
  public:
    VulkanDevice(const std::unique_ptr<VulkanInstance>& instance,
                 const VulkanPhysicalDevice::Requirements& requirements);
    ~VulkanDevice();

    [[nodiscard]] std::shared_ptr<VulkanQueue> get_graphics_queue() const { return m_graphics_queue; }
    [[nodiscard]] std::shared_ptr<VulkanQueue> get_presentation_queue() const { return m_presentation_queue; }

    [[nodiscard]] VkDevice handle() const { return m_device; }
    [[nodiscard]] VulkanPhysicalDevice physical_device() const { return m_physical_device; }

  private:
    VkDevice m_device{};
    VulkanPhysicalDevice m_physical_device;

    std::shared_ptr<VulkanQueue> m_graphics_queue;
    std::shared_ptr<VulkanQueue> m_presentation_queue;

    [[nodiscard]] VulkanPhysicalDevice select_physical_device(const std::unique_ptr<VulkanInstance>& instance,
                                                              const VulkanPhysicalDevice::Requirements& reqs) const;
};
