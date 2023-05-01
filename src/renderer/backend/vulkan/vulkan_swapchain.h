#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

namespace Phos {

// Forward declarations
class VulkanDevice;
class VulkanFramebuffer;
class VulkanRenderPass;
class VulkanImage;
class Window;

class VulkanSwapchain {
  public:
    explicit VulkanSwapchain(std::shared_ptr<VulkanRenderPass> render_pass);
    ~VulkanSwapchain();

    void acquire_next_image(VkSemaphore semaphore);
    [[nodiscard]] const std::unique_ptr<VulkanFramebuffer>& get_current_framebuffer() const {
        return m_framebuffers[m_current_image_idx];
    }
    [[nodiscard]] uint32_t get_current_image_idx() const { return m_current_image_idx; }

    void recreate();

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

    std::unique_ptr<VulkanImage> m_depth_image;
    VkImageView m_depth_image_view;

    // Current frame members
    uint32_t m_current_image_idx{};

    // Reference members
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};
    std::shared_ptr<VulkanRenderPass> m_render_pass;

    // Private methods
    void create();
    void cleanup();

    [[nodiscard]] SwapchainInformation get_swapchain_information() const;
    void retrieve_swapchain_images();
    void create_framebuffers();
};

} // namespace Phos
