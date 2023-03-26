#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <memory>

// Forward declarations
class VulkanDevice;

class VulkanSwapchain {
  public:
    VulkanSwapchain(std::shared_ptr<VulkanDevice> device, VkSurfaceKHR surface);
    ~VulkanSwapchain();

  private:
    struct SwapchainInformation {
        VkSurfaceCapabilitiesKHR capabilities;
        VkSurfaceFormatKHR surface_format;
        VkPresentModeKHR present_mode;
        VkExtent2D extent;
    };

    VkSwapchainKHR m_swapchain{};
    SwapchainInformation m_swapchain_info{};

    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_image_views;

    std::shared_ptr<VulkanDevice> m_device;
    VkSurfaceKHR m_surface;

    [[nodiscard]] SwapchainInformation get_swapchain_information() const;
    void retrieve_swapchain_images();
};
