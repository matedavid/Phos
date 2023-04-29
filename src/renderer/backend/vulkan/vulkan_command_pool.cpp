#include "vulkan_command_pool.h"

#include "renderer/backend/vulkan/vulkan_command_buffer.h"

namespace Phos {

VulkanCommandPool::VulkanCommandPool(VkDevice raw_device, uint32_t queue_family)
      : m_queue_family(queue_family), m_raw_device(raw_device) {
    VkCommandPoolCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    create_info.queueFamilyIndex = queue_family;

    VK_CHECK(vkCreateCommandPool(m_raw_device, &create_info, nullptr, &m_command_pool))
}

VulkanCommandPool::~VulkanCommandPool() {
    vkDestroyCommandPool(m_raw_device, m_command_pool, nullptr);
}

std::vector<std::shared_ptr<VulkanCommandBuffer>> VulkanCommandPool::allocate(uint32_t count) const {
    VkCommandBufferAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool = m_command_pool;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = count;

    std::vector<VkCommandBuffer> raw_command_buffers(count);
    VK_CHECK(vkAllocateCommandBuffers(m_raw_device, &allocate_info, raw_command_buffers.data()))

    std::vector<std::shared_ptr<VulkanCommandBuffer>> command_buffers{};
    for (const auto& command_buffer : raw_command_buffers) {
        command_buffers.push_back(std::make_shared<VulkanCommandBuffer>(command_buffer));
    }

    return command_buffers;
}

void VulkanCommandPool::free_command_buffer(const std::shared_ptr<VulkanCommandBuffer>& command_buffer) const {
    const std::array<VkCommandBuffer, 1> command_buffers = {command_buffer->handle()};
    vkFreeCommandBuffers(m_raw_device, m_command_pool, 1, command_buffers.data());
}

} // namespace Phos
