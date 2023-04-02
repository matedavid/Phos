#include "vulkan_command_pool.h"

#include "renderer/vulkan_device.h"
#include "renderer/vulkan_command_buffer.h"

VulkanCommandPool::VulkanCommandPool(std::shared_ptr<VulkanDevice> device,
                                     uint32_t queue_family,
                                     uint32_t command_buffer_count)
      : m_queue_family(queue_family), m_device(std::move(device)) {
    VkCommandPoolCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    create_info.queueFamilyIndex = queue_family;

    VK_CHECK(vkCreateCommandPool(m_device->handle(), &create_info, nullptr, &m_command_pool))

    // Allocate command buffers
    VkCommandBufferAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool = m_command_pool;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = command_buffer_count;

    std::vector<VkCommandBuffer> command_buffers(command_buffer_count);
    VK_CHECK(vkAllocateCommandBuffers(m_device->handle(), &allocate_info, command_buffers.data()))

    for (const auto& command_buffer : command_buffers) {
        m_command_buffers.push_back(std::make_shared<VulkanCommandBuffer>(command_buffer));
    }
}

VulkanCommandPool::~VulkanCommandPool() {
    vkDestroyCommandPool(m_device->handle(), m_command_pool, nullptr);
}
