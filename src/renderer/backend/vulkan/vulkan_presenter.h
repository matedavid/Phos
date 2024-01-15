#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

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
    std::vector<std::shared_ptr<VulkanCommandBuffer>> m_command_buffers;

    // Presentation pass
    std::shared_ptr<VulkanGraphicsPipeline> m_presentation_pipeline;
    std::shared_ptr<VulkanRenderPass> m_presentation_pass;

    // Synchronization
    std::vector<VkSemaphore> m_image_available_semaphores;
    std::vector<VkSemaphore> m_rendering_finished_semaphores;
    std::vector<VkFence> m_wait_fences;

    uint32_t m_current_frame = 0;

    void init();
};

} // namespace Phos
