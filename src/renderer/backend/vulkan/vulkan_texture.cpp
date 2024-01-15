#include "vulkan_texture.h"

#include <stb_image.h>

#include "vk_core.h"

#include "utility/logging.h"

#include "renderer/backend/vulkan/vulkan_device.h"
#include "renderer/backend/vulkan/vulkan_buffer.h"
#include "renderer/backend/vulkan/vulkan_image.h"
#include "renderer/backend/vulkan/vulkan_context.h"

namespace Phos {

VulkanTexture::VulkanTexture(const std::string& path) {
    // Load image
    int32_t width, height, channels, type_size;
    void* pixels;
    Image::Format format;

    if (stbi_is_hdr(path.c_str())) {
        pixels = stbi_loadf(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        type_size = sizeof(float);
        format = Image::Format::R32G32B32A32_SFLOAT;
    } else {
        pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        type_size = sizeof(stbi_uc);
        format = Image::Format::R8G8B8A8_SRGB;
    }

    if (!pixels) {
        PHOS_LOG_ERROR("Failed to load image: {}", path);
        return;
    }

    const auto image_size = static_cast<uint32_t>(width * height * 4 * type_size);

    const auto staging_buffer = VulkanBuffer{
        image_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };

    staging_buffer.copy_data(pixels);

    stbi_image_free(pixels);

    // Create image
    const auto description = VulkanImage::Description{
        .width = static_cast<uint32_t>(width),
        .height = static_cast<uint32_t>(height),
        .type = VulkanImage::Type::Image2D,
        .format = format,
        .transfer = true,
    };
    m_image = std::make_shared<VulkanImage>(description);

    // Transition image layout for copying
    m_image->transition_layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // Copy staging buffer to image
    staging_buffer.copy_to_image(*m_image);

    // Transition image layout for shader access
    m_image->transition_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

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

    VK_CHECK(vkCreateSampler(VulkanContext::device->handle(), &sampler_info, nullptr, &m_sampler));
}

VulkanTexture::VulkanTexture(uint32_t width, uint32_t height) {
    // Create image
    const auto description = VulkanImage::Description{
        .width = width,
        .height = height,
        .type = VulkanImage::Type::Image2D,
        .format = VulkanImage::Format::B8G8R8A8_SRGB,
        .attachment = true,
    };
    m_image = std::make_shared<VulkanImage>(description);

    // Transition image layout
    // m_image->transition_layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

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

    VK_CHECK(vkCreateSampler(VulkanContext::device->handle(), &sampler_info, nullptr, &m_sampler));
}

VulkanTexture::VulkanTexture(const std::vector<char>& data, uint32_t width, uint32_t height) {
    const auto image_size = width * height * 4;

    if (image_size != data.size())
        PHOS_LOG_WARNING("Size of data is not the same as the provided image size (width * height * 4)");

    const auto staging_buffer = VulkanBuffer{
        image_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };

    staging_buffer.copy_data(data.data());

    // Create image
    const auto description = VulkanImage::Description{
        .width = static_cast<uint32_t>(width),
        .height = static_cast<uint32_t>(height),
        .type = VulkanImage::Type::Image2D,
        .format = VulkanImage::Format::R8G8B8A8_SRGB,
        .transfer = true,
    };
    m_image = std::make_shared<VulkanImage>(description);

    // Transition image layout for copying
    m_image->transition_layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // Copy staging buffer to image
    staging_buffer.copy_to_image(*m_image);

    // Transition image layout for shader access
    m_image->transition_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

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

    VK_CHECK(vkCreateSampler(VulkanContext::device->handle(), &sampler_info, nullptr, &m_sampler));
}

VulkanTexture::VulkanTexture(const std::shared_ptr<Image>& image) {
    m_image = std::dynamic_pointer_cast<VulkanImage>(image);

    // @TODO: UGLY :)
    if (m_image->description().storage)
        m_image->transition_layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

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

    VK_CHECK(vkCreateSampler(VulkanContext::device->handle(), &sampler_info, nullptr, &m_sampler));
}

VulkanTexture::~VulkanTexture() {
    vkDestroySampler(VulkanContext::device->handle(), m_sampler, nullptr);
}

std::shared_ptr<Image> VulkanTexture::get_image() const {
    return std::dynamic_pointer_cast<Image>(m_image);
}

} // namespace Phos
