#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

// Forward declarations
class VulkanDevice;
class VulkanFramebuffer;
class VulkanRenderPass;
class Window;

class VulkanSwapchain {
  public:
    VulkanSwapchain(std::shared_ptr<VulkanDevice> device,
                    VkSurfaceKHR surface,
                    std::shared_ptr<Window> window,
                    std::shared_ptr<VulkanRenderPass> render_pass);
    ~VulkanSwapchain();

    void acquire_next_image(VkSemaphore semaphore);
    [[nodiscard]] const std::unique_ptr<VulkanFramebuffer>& get_current_framebuffer() const {
        return m_framebuffers[m_current_image_idx];
    }
    [[nodiscard]] uint32_t get_current_image_idx() const { return m_current_image_idx; }

    [[nodiscard]] VkSwapchainKHR handle() { return m_swapchain; }

  private:
    struct SwapchainInformation {
        VkSurfaceCapabilitiesKHR capabilities;
        VkSurfaceFormatKHR surface_format;
        VkPresentModeKHR present_mode;
        VkExtent2D extent;
    };
    SwapchainInformation m_swapchain_info{};

    // Swapchain specific members
    VkSwapchainKHR m_swapchain{};
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_image_views;
    std::vector<std::unique_ptr<VulkanFramebuffer>> m_framebuffers;

    // Current frame members
    uint32_t m_current_image_idx;

    // Reference members
    std::shared_ptr<VulkanDevice> m_device;
    VkSurfaceKHR m_surface;
    std::shared_ptr<Window> m_window;
    std::shared_ptr<VulkanRenderPass> m_render_pass;

    // Private methods
    [[nodiscard]] SwapchainInformation get_swapchain_information() const;
    void retrieve_swapchain_images();
    void create_framebuffers();
};
