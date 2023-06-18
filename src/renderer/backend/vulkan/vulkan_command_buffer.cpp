#include "vulkan_command_buffer.h"

#include "renderer/backend/vulkan/vulkan_context.h"
#include "renderer/backend/vulkan/vulkan_device.h"

namespace Phos {

VulkanCommandBuffer::VulkanCommandBuffer() : VulkanCommandBuffer(VulkanQueue::Type::Graphics) {}

VulkanCommandBuffer::VulkanCommandBuffer(VulkanQueue::Type type) : m_type(type) {
    const auto& device = VulkanContext::device;
    m_command_buffer = device->create_command_buffer(m_type);
}

VulkanCommandBuffer::~VulkanCommandBuffer() {
    const auto& device = VulkanContext::device;
    device->free_command_buffer(m_command_buffer, m_type);
}

void VulkanCommandBuffer::record(const std::function<void(void)>& func) const {
    // Begin command buffer
    begin(false);

    // Call command buffer function
    func();

    // End command buffer
    VK_CHECK(vkEndCommandBuffer(m_command_buffer))
}

void VulkanCommandBuffer::submit_single_time(
    VulkanQueue::Type type,
    const std::function<void(const std::shared_ptr<VulkanCommandBuffer>&)>& func) {
    const auto command_buffer = std::make_shared<VulkanCommandBuffer>(type);

    // Begin single time command buffer
    command_buffer->begin(true);

    // Call command buffer function
    func(command_buffer);

    // End command buffer
    command_buffer->end();

    // Submit the command buffer
    const std::vector<VkCommandBuffer> command_buffers = {command_buffer->handle()};

    VkSubmitInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());
    info.pCommandBuffers = command_buffers.data();

    const auto& queue = VulkanContext::device->get_queue_from_type(type);
    queue->submit(info, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue->handle());
}

void VulkanCommandBuffer::begin(bool one_time) const {
    VK_CHECK(vkResetCommandBuffer(m_command_buffer, 0))

    VkCommandBufferBeginInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (one_time)
        info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(m_command_buffer, &info))
}

void VulkanCommandBuffer::end() const {
    VK_CHECK(vkEndCommandBuffer(m_command_buffer))
}

} // namespace Phos
