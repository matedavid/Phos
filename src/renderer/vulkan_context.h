#pragma once

#include "vk_core.h"

#include "renderer/vulkan_instance.h"
#include "renderer/vulkan_physical_device.h"
#include "renderer/vulkan_swapchain.h"
#include "renderer/vulkan_device.h"

// Forward declarations
class Window;

class VulkanContext {
  public:
    explicit VulkanContext(std::shared_ptr<Window>& window);
    ~VulkanContext() = default;

  private:
    std::unique_ptr<VulkanInstance> m_instance;
    std::shared_ptr<VulkanSwapchain> m_swapchain;
    std::shared_ptr<VulkanDevice> m_device;

    [[nodiscard]] VulkanPhysicalDevice select_physical_device(
        const std::vector<VulkanPhysicalDevice>& physical_devices,
        const std::vector<const char*>& extensions) const;
};
