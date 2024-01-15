#include "vulkan_queue.h"

#include <array>

#include "vk_core.h"

#include "renderer/backend/vulkan/vulkan_command_buffer.h"
#include "renderer/backend/vulkan/vulkan_swapchain.h"

namespace Phos {

VulkanQueue::VulkanQueue(VkQueue queue, uint32_t queue_family) : m_queue(queue), m_queue_family(queue_family) {}

void VulkanQueue::submit(VkSubmitInfo info, VkFence fence) const {
    VK_CHECK(vkQueueSubmit(m_queue, 1, &info, fence));
}

VkResult VulkanQueue::submitKHR(const std::shared_ptr<VulkanSwapchain>& swapchain,
                                uint32_t image_index,
                                const std::vector<VkSemaphore>& wait_semaphores) const {
    const std::array<VkSwapchainKHR, 1> swapchains = {swapchain->handle()};

    // Submit result to the swap chain
    VkPresentInfoKHR info{};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = static_cast<uint32_t>(wait_semaphores.size());
    info.pWaitSemaphores = wait_semaphores.data();
    info.swapchainCount = 1;
    info.pSwapchains = swapchains.data();
    info.pImageIndices = &image_index;

    return vkQueuePresentKHR(m_queue, &info);
}

} // namespace Phos
