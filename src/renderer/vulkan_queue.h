#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

// Forward declarations
class VulkanCommandBuffer;
class VulkanSwapchain;

class VulkanQueue {
  public:
    enum class Type {
        Graphics,
        Compute,
        Transfer,
        Presentation
    };

    explicit VulkanQueue(VkQueue queue);
    ~VulkanQueue() = default;

    void submit(const std::shared_ptr<VulkanCommandBuffer>& command_buffer,
                const std::vector<VkSemaphore>& wait_semaphores,
                const std::vector<VkPipelineStageFlags>& wait_stages,
                const std::vector<VkSemaphore>& signal_semaphores,
                VkFence fence);

    VkResult submitKHR(const std::shared_ptr<VulkanSwapchain>& swapchain,
                       uint32_t image_index,
                       const std::vector<VkSemaphore>& wait_semaphores);

  private:
    VkQueue m_queue;
};
