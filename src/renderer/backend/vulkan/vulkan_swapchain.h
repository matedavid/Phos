#pragma once

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
    VulkanSwapchain();
    ~VulkanSwapchain();

    // void specify_render_pass(std::shared_ptr<VulkanRenderPass> render_pass);

    void acquire_next_image(VkSemaphore semaphore, VkFence fence);
    [[nodiscard]] const std::shared_ptr<VulkanFramebuffer>& get_current_framebuffer() const {
        return m_framebuffers[m_current_image_idx];
    }
    [[nodiscard]] uint32_t get_current_image_idx() const { return m_current_image_idx; }

    void recreate();

    [[nodiscard]] VkRenderPass get_render_pass() { return m_render_pass; };
    [[nodiscard]] std::shared_ptr<VulkanFramebuffer> get_target_framebuffer() const { return m_framebuffers[0]; }

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
    std::vector<std::shared_ptr<VulkanImage>> m_images;
    std::vector<std::shared_ptr<VulkanFramebuffer>> m_framebuffers;

    VkRenderPass m_render_pass{VK_NULL_HANDLE};

    std::shared_ptr<VulkanImage> m_depth_image;

    // Current frame members
    uint32_t m_current_image_idx{};

    // Reference members
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};

    // Private methods
    void create();
    void cleanup();

    [[nodiscard]] SwapchainInformation get_swapchain_information() const;
    void retrieve_swapchain_images();
    void create_framebuffers();

    void create_render_pass();
};

} // namespace Phos
