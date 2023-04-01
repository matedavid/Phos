#include "vulkan_buffers.h"

#include "renderer/vulkan_device.h"
#include "renderer/vulkan_command_buffer.h"

VulkanVertexBuffer::VulkanVertexBuffer(std::shared_ptr<VulkanDevice> device, const std::vector<float>& data)
    : m_device(std::move(device)) {
    VkBufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = data.size() * sizeof(float);
    create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(m_device->handle(), &create_info, nullptr, &m_buffer))

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(m_device->handle(), m_buffer, &memory_requirements);

    const auto memory_type_index = find_memory_type(
        memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    CORE_ASSERT(memory_type_index.has_value(), "No suitable memory to allocate vertex buffer")

    VkMemoryAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = memory_type_index.value();

    // Allocate memory
    VK_CHECK(vkAllocateMemory(m_device->handle(), &allocate_info, nullptr, &m_memory))

    // Bind memory to buffer
    VK_CHECK(vkBindBufferMemory(m_device->handle(), m_buffer, m_memory, 0))

    void* map_data;
    VK_CHECK(vkMapMemory(m_device->handle(), m_memory, 0, create_info.size, 0, &map_data))
    memcpy(map_data, data.data(), (size_t)create_info.size);
    vkUnmapMemory(m_device->handle(), m_memory);
}

VulkanVertexBuffer::~VulkanVertexBuffer() {
    vkDestroyBuffer(m_device->handle(), m_buffer, nullptr);
    vkFreeMemory(m_device->handle(), m_memory, nullptr);
}

void VulkanVertexBuffer::bind(const std::shared_ptr<VulkanCommandBuffer>& command_buffer) const {
    std::array<VkBuffer, 1> vertex_buffers = {m_buffer};
    VkDeviceSize offsets[] = {0};

    vkCmdBindVertexBuffers(command_buffer->handle(), 0, vertex_buffers.size(), vertex_buffers.data(), offsets);
}

std::optional<uint32_t> VulkanVertexBuffer::find_memory_type(uint32_t filter, VkMemoryPropertyFlags properties) const {
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(m_device->physical_device().handle(), &memory_properties);

    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i) {
        if (filter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    return {};
}