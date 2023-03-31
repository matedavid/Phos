#include "vulkan_command_buffer.h"

VulkanCommandBuffer::VulkanCommandBuffer(VkCommandBuffer command_buffer) : m_command_buffer(command_buffer) {
}

void VulkanCommandBuffer::begin() {
    CORE_ASSERT(!m_recording, "Can't begin a new command buffer while recording another one")
    m_recording = true;

    VK_CHECK(vkResetCommandBuffer(m_command_buffer, 0))

    VkCommandBufferBeginInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VK_CHECK(vkBeginCommandBuffer(m_command_buffer, &info))
}

void VulkanCommandBuffer::end() {
    CORE_ASSERT(m_recording, "Can't end a command buffer while not recording")
    m_recording = false;

    VK_CHECK(vkEndCommandBuffer(m_command_buffer));
}