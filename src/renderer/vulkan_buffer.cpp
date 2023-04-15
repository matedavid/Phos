#include "vulkan_buffer.h"

#include "renderer/vulkan_device.h"
#include "renderer/vulkan_command_buffer.h"
#include "renderer/vulkan_image.h"

VulkanBuffer::VulkanBuffer(std::shared_ptr<VulkanDevice> device,
                           VkDeviceSize size,
                           VkBufferUsageFlags usage,
                           VkMemoryPropertyFlags properties)
      : m_size(size), m_device(std::move(device)) {
    // Create buffer
    VkBufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = size;
    create_info.usage = usage;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(m_device->handle(), &create_info, nullptr, &m_buffer))

    // Allocate memory
    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(m_device->handle(), m_buffer, &memory_requirements);

    const auto memory_type_index =
        m_device->physical_device().find_memory_type(memory_requirements.memoryTypeBits, properties);
    CORE_ASSERT(memory_type_index.has_value(), "No suitable memory to allocate buffer")

    VkMemoryAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = memory_type_index.value();

    VK_CHECK(vkAllocateMemory(m_device->handle(), &allocate_info, nullptr, &m_memory))

    // Bind memory to buffer
    VK_CHECK(vkBindBufferMemory(m_device->handle(), m_buffer, m_memory, 0))
}

VulkanBuffer::~VulkanBuffer() {
    vkDestroyBuffer(m_device->handle(), m_buffer, nullptr);
    vkFreeMemory(m_device->handle(), m_memory, nullptr);
}

void VulkanBuffer::map_memory(void*& memory) const {
    VK_CHECK(vkMapMemory(m_device->handle(), m_memory, 0, m_size, 0, &memory));
}

void VulkanBuffer::unmap_memory() const {
    vkUnmapMemory(m_device->handle(), m_memory);
}

void VulkanBuffer::copy_data(const void* data) const {
    void* mem;
    map_memory(mem);
    memcpy(mem, data, m_size);
    unmap_memory();
}

void VulkanBuffer::copy_to_buffer(const VulkanBuffer& buffer) const {
    CORE_ASSERT(m_size == buffer.m_size, "Size of buffers do not match")

    // TODO: Maybe should use transfer queue instead of graphics
    const auto& graphics_queue = m_device->get_graphics_queue();
    const auto command_buffer = m_device->create_command_buffer(VulkanQueue::Type::Graphics);

    command_buffer->begin(true);

    VkBufferCopy copy{};
    copy.srcOffset = 0;
    copy.dstOffset = 0;
    copy.size = m_size;

    vkCmdCopyBuffer(command_buffer->handle(), m_buffer, buffer.handle(), 1, &copy);

    command_buffer->end();

    // Submit queue

    // Submit queue
    const std::array<VkCommandBuffer, 1> command_buffers = {command_buffer->handle()};

    VkSubmitInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.commandBufferCount = 1;
    info.pCommandBuffers = command_buffers.data();

    graphics_queue->submit(info, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue->handle());

    // Free command buffer
    m_device->free_command_buffer(command_buffer, VulkanQueue::Type::Graphics);
}

void VulkanBuffer::copy_to_image(const VulkanImage& image) const {
    const auto command_buffer = m_device->create_command_buffer(VulkanQueue::Type::Graphics);

    command_buffer->begin(true);

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
        command_buffer->handle(), m_buffer, image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    command_buffer->end();

    // Submit
    const std::array<VkCommandBuffer, 1> command_buffers = {command_buffer->handle()};

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = command_buffers.data();

    m_device->get_graphics_queue()->submit(submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_device->get_graphics_queue()->handle());

    // Free command buffer
    m_device->free_command_buffer(command_buffer, VulkanQueue::Type::Graphics);
}
