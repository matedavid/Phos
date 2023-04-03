#include "vulkan_queue.h"

#include <array>

#include "renderer/vulkan_command_buffer.h"
#include "renderer/vulkan_swapchain.h"

VulkanQueue::VulkanQueue(VkQueue queue) : m_queue(queue) {}

void VulkanQueue::submit(const std::shared_ptr<VulkanCommandBuffer>& command_buffer,
                         const std::vector<VkSemaphore>& wait_semaphores,
                         const std::vector<VkPipelineStageFlags>& wait_stages,
                         const std::vector<VkSemaphore>& signal_semaphores,
                         VkFence fence) {
    const std::array<VkCommandBuffer, 1> command_buffers = {command_buffer->handle()};

    VkSubmitInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.waitSemaphoreCount = (uint32_t)wait_semaphores.size();
    info.pWaitSemaphores = wait_semaphores.data();
    info.pWaitDstStageMask = wait_stages.data();
    info.commandBufferCount = 1;
    info.pCommandBuffers = command_buffers.data();
    info.signalSemaphoreCount = (uint32_t)signal_semaphores.size();
    info.pSignalSemaphores = signal_semaphores.data();

    VK_CHECK(vkQueueSubmit(m_queue, 1, &info, fence))
}

VkResult VulkanQueue::submitKHR(const std::shared_ptr<VulkanSwapchain>& swapchain,
                                uint32_t image_index,
                                const std::vector<VkSemaphore>& wait_semaphores) {
    const std::array<VkSwapchainKHR, 1> swapchains = {swapchain->handle()};

    // Submit result to the swap chain
    VkPresentInfoKHR info{};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = (uint32_t)wait_semaphores.size();
    info.pWaitSemaphores = wait_semaphores.data();
    info.swapchainCount = 1;
    info.pSwapchains = swapchains.data();
    info.pImageIndices = &image_index;

    return vkQueuePresentKHR(m_queue, &info);
}
