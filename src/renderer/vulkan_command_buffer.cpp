#include "vulkan_command_buffer.h"

namespace Phos {

VulkanCommandBuffer::VulkanCommandBuffer(VkCommandBuffer command_buffer) : m_command_buffer(command_buffer) {}

void VulkanCommandBuffer::record(const std::function<void(void)>& func) const {
    // Begin command buffer
    begin(false);

    // Call command buffer function
    func();

    // End command buffer
    VK_CHECK(vkEndCommandBuffer(m_command_buffer))
}

void VulkanCommandBuffer::record_single_time(const std::function<void(const VulkanCommandBuffer&)>& func) const {
    // Begin single time command buffer
    begin(true);

    // Call command buffer function
    func(*this);

    // End command buffer
    VK_CHECK(vkEndCommandBuffer(m_command_buffer))
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
