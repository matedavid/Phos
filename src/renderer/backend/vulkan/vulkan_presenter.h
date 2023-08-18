#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>

#include "renderer/backend/presenter.h"

namespace Phos {

// Forward declarations
class VulkanSwapchain;
class VulkanGraphicsPipeline;
class VulkanRenderPass;
class VulkanCommandBuffer;
class VulkanQueue;

class VulkanPresenter : public Presenter {
  public:
    VulkanPresenter(std::shared_ptr<ISceneRenderer> renderer, std::shared_ptr<Window> window);
    ~VulkanPresenter() override;

    void present() override;
    void window_resized(uint32_t width, uint32_t height) override;

  private:
    std::shared_ptr<ISceneRenderer> m_renderer;
    std::shared_ptr<Window> m_window;
    std::shared_ptr<VulkanSwapchain> m_swapchain;

    std::shared_ptr<VulkanQueue> m_graphics_queue;
    std::shared_ptr<VulkanQueue> m_presentation_queue;
    std::shared_ptr<VulkanCommandBuffer> m_command_buffer;

    // Presentation pass
    std::shared_ptr<VulkanGraphicsPipeline> m_presentation_pipeline;
    std::shared_ptr<VulkanRenderPass> m_presentation_pass;

    // Synchronization
    VkSemaphore m_image_available_semaphore{VK_NULL_HANDLE};
    VkSemaphore m_rendering_finished_semaphore{VK_NULL_HANDLE};
    VkFence m_wait_fence{VK_NULL_HANDLE};

    void init();
};

} // namespace Phos
