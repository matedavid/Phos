#include "vulkan_skybox.h"

#include <filesystem>
#include <stb_image.h>

#include "renderer/backend/vulkan/vulkan_image.h"
#include "renderer/backend/vulkan/vulkan_buffer.h"
#include "renderer/backend/vulkan/vulkan_context.h"
#include "renderer/backend/vulkan/vulkan_device.h"

namespace Phos {

VulkanSkybox::VulkanSkybox(const Sides& sides) {
    init(sides);
}

VulkanSkybox::VulkanSkybox(const Sides& sides, const std::string& directory) {
    const auto dir = std::filesystem::path(directory);

    const auto directory_sides = Skybox::Sides{
        .front = dir / sides.front,
        .back = dir / sides.back,
        .up = dir / sides.up,
        .down = dir / sides.down,
        .left = dir / sides.left,
        .right = dir / sides.right,
    };

    init(directory_sides);
}

void VulkanSkybox::init(const Sides& sides) {
    int32_t width, height;

    std::array<std::vector<char>, 6> side_data;
    load_side(sides.front, side_data[0], width, height);
    load_side(sides.back, side_data[1], width, height);
    load_side(sides.up, side_data[2], width, height);
    load_side(sides.down, side_data[3], width, height);
    load_side(sides.left, side_data[4], width, height);
    load_side(sides.right, side_data[5], width, height);

    const auto image_size = static_cast<uint32_t>(width * height * 4) * 6;

    std::vector<char> data;
    std::ranges::copy(side_data[0], std::back_inserter(data));
    std::ranges::copy(side_data[1], std::back_inserter(data));
    std::ranges::copy(side_data[2], std::back_inserter(data));
    std::ranges::copy(side_data[3], std::back_inserter(data));
    std::ranges::copy(side_data[4], std::back_inserter(data));
    std::ranges::copy(side_data[5], std::back_inserter(data));

    PS_ASSERT(data.size() == image_size, "Size of skybox data does not match expected of size: {}", image_size)

    m_image = std::make_unique<VulkanImage>(VulkanImage::Description{
        .width = static_cast<uint32_t>(width),
        .height = static_cast<uint32_t>(height),
        .type = VulkanImage::Type::Cubemap,
        .format = VulkanImage::Format::R8G8B8A8_SRGB,
        .num_layers = 6,
        .transfer = true,
    });

    const auto staging_buffer = VulkanBuffer{
        image_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };
    staging_buffer.copy_data(data.data());

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

    VK_CHECK(vkCreateSampler(VulkanContext::device->handle(), &sampler_info, nullptr, &m_sampler))
}

void VulkanSkybox::load_side(const std::string& path, std::vector<char>& data, int32_t& width, int32_t& height) {
    int32_t channels;
    stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    const auto image_size = static_cast<uint32_t>(width * height * 4);
    data.resize(image_size);

    memcpy(data.data(), pixels, image_size);

    stbi_image_free(pixels);
}

VkImageView VulkanSkybox::view() const {
    return m_image->view();
}

VkSampler VulkanSkybox::sampler() const {
    return m_sampler;
}

} // namespace Phos
