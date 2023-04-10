#include "vulkan_buffers.h"

std::pair<VkBuffer, VkDeviceMemory> BufferUtils::create_buffer(const std::shared_ptr<VulkanDevice>& device,
                                                               VkDeviceSize size,
                                                               VkBufferUsageFlags usage,
                                                               VkMemoryPropertyFlags properties) {
    VkBufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = size;
    create_info.usage = usage;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer buffer;
    VK_CHECK(vkCreateBuffer(device->handle(), &create_info, nullptr, &buffer))

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(device->handle(), buffer, &memory_requirements);

    const auto memory_type_index =
        find_memory_type(device->physical_device().handle(), memory_requirements.memoryTypeBits, properties);

    CORE_ASSERT(memory_type_index.has_value(), "No suitable memory to allocate vertex buffer")

    VkMemoryAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = memory_type_index.value();

    // Allocate memory
    VkDeviceMemory memory;
    VK_CHECK(vkAllocateMemory(device->handle(), &allocate_info, nullptr, &memory))

    // Bind memory to buffer
    VK_CHECK(vkBindBufferMemory(device->handle(), buffer, memory, 0))

    return {buffer, memory};
}

std::optional<uint32_t> BufferUtils::find_memory_type(VkPhysicalDevice device,
                                                      uint32_t filter,
                                                      VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(device, &memory_properties);

    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i) {
        if (filter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    return {};
}

//
// Index Buffer
//
VulkanIndexBuffer::VulkanIndexBuffer(std::shared_ptr<VulkanDevice> device, const std::vector<uint32_t>& indices)
      : m_count((uint32_t)indices.size()), m_device(std::move(device)) {
    const VkDeviceSize size = indices.size() * sizeof(uint32_t);

    const auto& [staging_buffer, staging_buffer_memory] =
        BufferUtils::create_buffer(m_device,
                                   size,
                                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* map_data;
    VK_CHECK(vkMapMemory(m_device->handle(), staging_buffer_memory, 0, size, 0, &map_data))
    memcpy(map_data, indices.data(), (size_t)size);
    vkUnmapMemory(m_device->handle(), staging_buffer_memory);

    std::tie(m_buffer, m_memory) =
        BufferUtils::create_buffer(m_device,
                                   size,
                                   VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    BufferUtils::copy_buffer(m_device, staging_buffer, m_buffer, size);

    // Destroy staging buffer
    vkDestroyBuffer(m_device->handle(), staging_buffer, nullptr);
    vkFreeMemory(m_device->handle(), staging_buffer_memory, nullptr);
}

VulkanIndexBuffer::~VulkanIndexBuffer() {
    vkDestroyBuffer(m_device->handle(), m_buffer, nullptr);
    vkFreeMemory(m_device->handle(), m_memory, nullptr);
}

void VulkanIndexBuffer::bind(const std::shared_ptr<VulkanCommandBuffer>& command_buffer) const {
    vkCmdBindIndexBuffer(command_buffer->handle(), m_buffer, 0, VK_INDEX_TYPE_UINT32);
}
