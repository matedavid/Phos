#include "vulkan_buffer.h"

#include "renderer/backend/vulkan/vulkan_device.h"
#include "renderer/backend/vulkan/vulkan_command_buffer.h"
#include "renderer/backend/vulkan/vulkan_image.h"
#include "renderer/backend/vulkan/vulkan_context.h"

namespace Phos {

VulkanBuffer::VulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
      : m_size(size) {
    // Create buffer
    VkBufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = size;
    create_info.usage = usage;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(VulkanContext::device->handle(), &create_info, nullptr, &m_buffer))

    // Allocate memory
    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(VulkanContext::device->handle(), m_buffer, &memory_requirements);

    const auto memory_type_index =
        VulkanContext::device->physical_device().find_memory_type(memory_requirements.memoryTypeBits, properties);
    PS_ASSERT(memory_type_index.has_value(), "No suitable memory to allocate buffer")

    VkMemoryAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = memory_type_index.value();

    VK_CHECK(vkAllocateMemory(VulkanContext::device->handle(), &allocate_info, nullptr, &m_memory))

    // Bind memory to buffer
    VK_CHECK(vkBindBufferMemory(VulkanContext::device->handle(), m_buffer, m_memory, 0))
}

VulkanBuffer::~VulkanBuffer() {
    vkDestroyBuffer(VulkanContext::device->handle(), m_buffer, nullptr);
    vkFreeMemory(VulkanContext::device->handle(), m_memory, nullptr);
}

void VulkanBuffer::map_memory(void*& memory) const {
    VK_CHECK(vkMapMemory(VulkanContext::device->handle(), m_memory, 0, m_size, 0, &memory));
}

void VulkanBuffer::unmap_memory() const {
    vkUnmapMemory(VulkanContext::device->handle(), m_memory);
}

void VulkanBuffer::copy_data(const void* data) const {
    void* mem;
    map_memory(mem);
    memcpy(mem, data, m_size);
    unmap_memory();
}

void VulkanBuffer::copy_to_buffer(const VulkanBuffer& buffer) const {
    PS_ASSERT(m_size == buffer.m_size, "Size of buffers do not match")

    // TODO: Maybe should use transfer queue instead of graphics
    VulkanContext::device->single_time_command_buffer(
        VulkanQueue::Type::Graphics, [&](const VulkanCommandBuffer& command_buffer) {
            VkBufferCopy copy{};
            copy.srcOffset = 0;
            copy.dstOffset = 0;
            copy.size = m_size;

            vkCmdCopyBuffer(command_buffer.handle(), m_buffer, buffer.handle(), 1, &copy);
        });
}

void VulkanBuffer::copy_to_image(const VulkanImage& image) const {
    // TODO: Maybe should use transfer queue instead of graphics
    VulkanContext::device->single_time_command_buffer(
        VulkanQueue::Type::Graphics, [&](const VulkanCommandBuffer& command_buffer) {
            VkBufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;

            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;

            region.imageOffset = {0, 0, 0};
            region.imageExtent = {image.width(), image.height(), 1};

            vkCmdCopyBufferToImage(
                command_buffer.handle(), m_buffer, image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        });
}

} // namespace Phos