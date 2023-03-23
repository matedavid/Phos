#pragma once

#include "vk_core.h"

#include "renderer/vulkan_instance.h"
#include "renderer/vulkan_physical_device.h"
#include "renderer/vulkan_device.h"

// Forward declarations
class GLFWwindow;

class VulkanContext {
  public:
    VulkanContext(const std::vector<const char*>& required_extensions, GLFWwindow* window);
    ~VulkanContext();

  private:
    std::unique_ptr<VulkanInstance> m_instance;
    VkSurfaceKHR m_surface;

    [[nodiscard]] VulkanPhysicalDevice select_physical_device(
        const std::vector<VulkanPhysicalDevice>& physical_devices,
        const std::vector<const char*>& extensions) const;
};
