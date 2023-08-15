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

    explicit VulkanQueue(VkQueue queue, uint32_t queue_family);
    ~VulkanQueue() = default;

    void submit(VkSubmitInfo info, VkFence fence);

    VkResult submitKHR(const std::shared_ptr<VulkanSwapchain>& swapchain,
                       uint32_t image_index,
                       const std::vector<VkSemaphore>& wait_semaphores);

    [[nodiscard]] VkQueue handle() const { return m_queue; }
    [[nodiscard]] uint32_t family() const { return m_queue_family; }

  private:
    VkQueue m_queue;
    uint32_t m_queue_family;
};

} // namespace Phos
