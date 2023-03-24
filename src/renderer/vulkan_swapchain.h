#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <memory>

// Forward declarations
class VulkanDevice;

class VulkanSwapchain {
  public:
    VulkanSwapchain(std::shared_ptr<VulkanDevice>& device, VkSurfaceKHR surface);
    ~VulkanSwapchain();

  private:
    VkSwapchainKHR m_swapchain;

    std::shared_ptr<VulkanDevice> m_device;
    VkSurfaceKHR m_surface;

    struct SwapchainInformation {
        VkSurfaceCapabilitiesKHR capabilities;
        VkSurfaceFormatKHR surface_format;
        VkPresentModeKHR present_mode;
        VkExtent2D extent;
    };

    SwapchainInformation get_swapchain_information() const;
};
