#include "vulkan_image.h"

#include "renderer/vulkan_device.h"
#include "renderer/vulkan_command_buffer.h"
#include "renderer/vulkan_context.h"

namespace Phos {

VulkanImage::VulkanImage(const Description& description) {
    VkImageCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.imageType = description.image_type;
    create_info.extent.width = description.width;
    create_info.extent.height = description.height;
    create_info.extent.depth = 1;
    create_info.mipLevels = 1;
    create_info.arrayLayers = 1;
    create_info.format = description.format;
    create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    create_info.initialLayout = description.initial_layout;
    create_info.usage = description.usage;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.samples = VK_SAMPLE_COUNT_1_BIT;

    VK_CHECK(vkCreateImage(VulkanContext::device->handle(), &create_info, nullptr, &m_image))

    m_width = description.width;
    m_height = description.height;

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(VulkanContext::device->handle(), m_image, &memory_requirements);

    const auto memory_type_index = VulkanContext::device->physical_device().find_memory_type(
        memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    PS_ASSERT(memory_type_index.has_value(), "No suitable memory to allocate image")

    VkMemoryAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = memory_type_index.value();

    VK_CHECK(vkAllocateMemory(VulkanContext::device->handle(), &allocate_info, nullptr, &m_memory))

    // Bind memory to image
    VK_CHECK(vkBindImageMemory(VulkanContext::device->handle(), m_image, m_memory, 0))
}

VulkanImage::~VulkanImage() {
    vkDestroyImage(VulkanContext::device->handle(), m_image, nullptr);
    vkFreeMemory(VulkanContext::device->handle(), m_memory, nullptr);
}

void VulkanImage::transition_layout(VkImageLayout old_layout, VkImageLayout new_layout) const {
    VulkanContext::device->single_time_command_buffer(
        VulkanQueue::Type::Graphics, [&](const VulkanCommandBuffer& command_buffer) {
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = old_layout;
            barrier.newLayout = new_layout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = m_image;
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
                PS_FAIL("Unsupported layout transition")
            }

            vkCmdPipelineBarrier(
                command_buffer.handle(), source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        });
}

} // namespace Phos
