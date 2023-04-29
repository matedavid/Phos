#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

namespace Phos {

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

    void submit(VkSubmitInfo info, VkFence fence);

    VkResult submitKHR(const std::shared_ptr<VulkanSwapchain>& swapchain,
                       uint32_t image_index,
                       const std::vector<VkSemaphore>& wait_semaphores);

    [[nodiscard]] VkQueue handle() const { return m_queue; }

  private:
    VkQueue m_queue;
};

} // namespace Phos
