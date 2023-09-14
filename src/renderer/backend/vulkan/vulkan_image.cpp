#include "vulkan_image.h"

#include "renderer/backend/vulkan/vulkan_device.h"
#include "renderer/backend/vulkan/vulkan_command_buffer.h"
#include "renderer/backend/vulkan/vulkan_context.h"

namespace Phos {

VulkanImage::VulkanImage(const Description& description) : m_description(description) {
    // Create image
    VkImageCreateInfo image_create_info{};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType = get_image_type(description.type);
    image_create_info.format = get_image_format(description.format);

    image_create_info.extent.width = description.width;
    image_create_info.extent.height = description.height;
    image_create_info.extent.depth = 1;

    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = description.num_layers;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    // Usage
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT; // TODO: Maybe bad as default?

    if (description.transfer)
        image_create_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    if (description.attachment && is_depth_format(description.format))
        image_create_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    else if (description.attachment)
        image_create_info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (description.storage)
        image_create_info.usage |= VK_IMAGE_USAGE_STORAGE_BIT;

    // Flags
    if (description.type == Image::Type::Cubemap)
        image_create_info.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    VK_CHECK(vkCreateImage(VulkanContext::device->handle(), &image_create_info, nullptr, &m_image))

    // Allocate image
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

    // Create image view
    create_image_view(description);
}

VulkanImage::VulkanImage(const Description& description, VkImage image) : m_image(image), m_description(description) {
    create_image_view(description);
    m_created_resources = false;
}

VulkanImage::~VulkanImage() {
    if (m_created_resources) {
        vkDestroyImage(VulkanContext::device->handle(), m_image, nullptr);
        vkFreeMemory(VulkanContext::device->handle(), m_memory, nullptr);
    }
    vkDestroyImageView(VulkanContext::device->handle(), m_image_view, nullptr);
}

void VulkanImage::transition_layout(VkImageLayout old_layout, VkImageLayout new_layout) const {
    VulkanCommandBuffer::submit_single_time(
        VulkanQueue::Type::Graphics, [&](const std::shared_ptr<VulkanCommandBuffer>& command_buffer) {
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
            barrier.subresourceRange.layerCount = m_description.num_layers;

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
            } else if (old_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
                       new_layout == VK_IMAGE_LAYOUT_GENERAL) {
                barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                source_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            } else {
                PS_FAIL("Unsupported layout transition")
            }

            vkCmdPipelineBarrier(
                command_buffer->handle(), source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        });
}

VkFormat VulkanImage::get_image_format(Format format) {
    switch (format) {
    default:
    case Format::B8G8R8A8_SRGB:
        return VK_FORMAT_B8G8R8A8_SRGB;
    case Format::R8G8B8A8_SRGB:
        return VK_FORMAT_R8G8B8A8_SRGB;
    case Format::R16G16B16A16_SFLOAT:
        return VK_FORMAT_R16G16B16A16_SFLOAT;
    case Format::D32_SFLOAT:
        return VK_FORMAT_D32_SFLOAT;
    }
}

VkImageType VulkanImage::get_image_type(Type type) {
    switch (type) {
    default:
    case Type::Image2D:
        return VK_IMAGE_TYPE_2D;
    case Type::Image3D:
        return VK_IMAGE_TYPE_3D;
    case Type::Cubemap:
        return VK_IMAGE_TYPE_2D;
    }
}

VkImageViewType VulkanImage::get_image_view_type(Type type) {
    switch (type) {
    default:
    case Type::Image2D:
        return VK_IMAGE_VIEW_TYPE_2D;
    case Type::Image3D:
        return VK_IMAGE_VIEW_TYPE_3D;
    case Type::Cubemap:
        return VK_IMAGE_VIEW_TYPE_CUBE;
    }
}

bool VulkanImage::is_depth_format(Format format) {
    return format == Format::D32_SFLOAT;
}

void VulkanImage::create_image_view(const Description& description) {
    // Create image view
    VkImageViewCreateInfo view_create_info{};
    view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_create_info.image = m_image;
    view_create_info.viewType = get_image_view_type(description.type);
    view_create_info.format = get_image_format(description.format);

    if (is_depth_format(description.format))
        view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    else
        view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    view_create_info.subresourceRange.baseMipLevel = 0;
    view_create_info.subresourceRange.levelCount = 1;
    view_create_info.subresourceRange.baseArrayLayer = 0;
    view_create_info.subresourceRange.layerCount = m_description.num_layers;

    VK_CHECK(vkCreateImageView(VulkanContext::device->handle(), &view_create_info, nullptr, &m_image_view))
}

} // namespace Phos
