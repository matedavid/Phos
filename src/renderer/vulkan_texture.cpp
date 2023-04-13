#include "vulkan_texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "renderer/vulkan_buffers.h"

VulkanTexture::VulkanTexture(std::shared_ptr<VulkanDevice> device, const std::string& path)
      : m_device(std::move(device)) {
    // Load image
    int32_t width, height, channels;
    stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    m_width = static_cast<uint32_t>(width);
    m_height = static_cast<uint32_t>(height);

    if (!pixels) {
        CORE_ERROR("Failed to load image: {}", path)
        return;
    }

    const VkDeviceSize image_size = m_width * m_height * 4;

    // Create buffer
    const auto& [staging_buffer, staging_buffer_memory] =
        BufferUtils::create_buffer(m_device,
                                   image_size,
                                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    vkMapMemory(m_device->handle(), staging_buffer_memory, 0, image_size, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(image_size));
    vkUnmapMemory(m_device->handle(), staging_buffer_memory);

    stbi_image_free(pixels);

    // Create image
    VkImageCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.imageType = VK_IMAGE_TYPE_2D;
    create_info.extent.width = static_cast<uint32_t>(width);
    create_info.extent.height = static_cast<uint32_t>(height);
    create_info.extent.depth = 1;
    create_info.mipLevels = 1;
    create_info.arrayLayers = 1;
    create_info.format = VK_FORMAT_R8G8B8A8_SRGB;
    create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.samples = VK_SAMPLE_COUNT_1_BIT;

    VK_CHECK(vkCreateImage(m_device->handle(), &create_info, nullptr, &m_image))

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(m_device->handle(), m_image, &memory_requirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memory_requirements.size;
    allocInfo.memoryTypeIndex =
        BufferUtils::find_memory_type(m_device->physical_device().handle(),
                                      memory_requirements.memoryTypeBits,
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
            .value();

    VK_CHECK(vkAllocateMemory(m_device->handle(), &allocInfo, nullptr, &m_image_memory))

    vkBindImageMemory(m_device->handle(), m_image, m_image_memory, 0);

    // Transition image layout
    transition_image_layout(m_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    // Copy buffer to image
    copy_buffer_to_image(staging_buffer);

    transition_image_layout(m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // Cleanup
    vkDestroyBuffer(m_device->handle(), staging_buffer, nullptr);
    vkFreeMemory(m_device->handle(), staging_buffer_memory, nullptr);

    //
    //
    //

    // Create Image View
    VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = m_image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = VK_FORMAT_R8G8B8A8_SRGB;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(m_device->handle(), &view_info, nullptr, &m_image_view))

    // Create sampler
    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = VK_FALSE;
    sampler_info.maxAnisotropy = 1.0f;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;

    VK_CHECK(vkCreateSampler(m_device->handle(), &sampler_info, nullptr, &m_sampler))
}

VulkanTexture::~VulkanTexture() {
    vkDestroySampler(m_device->handle(), m_sampler, nullptr);
    vkDestroyImageView(m_device->handle(), m_image_view, nullptr);

    vkDestroyImage(m_device->handle(), m_image, nullptr);
    vkFreeMemory(m_device->handle(), m_image_memory, nullptr);
}

void VulkanTexture::transition_image_layout(VkImage image, VkImageLayout old_layout, VkImageLayout new_layout) {
    const auto command_buffer = m_device->create_command_buffer(VulkanQueue::Type::Graphics);

    command_buffer->begin(true);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        CORE_FAIL("Unsupported layout transition")
    }

    vkCmdPipelineBarrier(
        command_buffer->handle(), source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

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

void VulkanTexture::copy_buffer_to_image(VkBuffer buffer) {
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
    region.imageExtent = {m_width, m_height, 1};

    vkCmdCopyBufferToImage(command_buffer->handle(), buffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

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
