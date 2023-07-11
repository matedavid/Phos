#include "vulkan_skybox.h"

#include <filesystem>
#include <stb_image.h>

#include "renderer/backend/vulkan/vulkan_image.h"
#include "renderer/backend/vulkan/vulkan_buffer.h"
#include "renderer/backend/vulkan/vulkan_context.h"
#include "renderer/backend/vulkan/vulkan_device.h"

namespace Phos {

VulkanSkybox::VulkanSkybox(const Faces& sides) {
    init(sides);
}

VulkanSkybox::VulkanSkybox(const Faces& faces, const std::string& directory) {
    const auto dir = std::filesystem::path(directory);

    const auto directory_sides = Skybox::Faces{
        .right = dir / faces.right,
        .left = dir / faces.left,
        .top = dir / faces.top,
        .bottom = dir / faces.bottom,
        .front = dir / faces.front,
        .back = dir / faces.back,
    };

    init(directory_sides);
}

VulkanSkybox::~VulkanSkybox() {
    vkDestroySampler(VulkanContext::device->handle(), m_sampler, nullptr);
}

void VulkanSkybox::init(const Faces& faces) {
    int32_t width, height;

    // Order from:
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap16.html#_cube_map_face_selection_and_transformations
    std::array<std::vector<char>, 6> face_data;
    load_face(faces.right, face_data[0], width, height);
    load_face(faces.left, face_data[1], width, height);
    load_face(faces.top, face_data[2], width, height);
    load_face(faces.bottom, face_data[3], width, height);
    load_face(faces.front, face_data[4], width, height);
    load_face(faces.back, face_data[5], width, height);

    const auto image_size = static_cast<uint32_t>(width * height * 4) * 6;

    std::vector<char> data;
    std::ranges::copy(face_data[0], std::back_inserter(data));
    std::ranges::copy(face_data[1], std::back_inserter(data));
    std::ranges::copy(face_data[2], std::back_inserter(data));
    std::ranges::copy(face_data[3], std::back_inserter(data));
    std::ranges::copy(face_data[4], std::back_inserter(data));
    std::ranges::copy(face_data[5], std::back_inserter(data));

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

void VulkanSkybox::load_face(const std::string& path, std::vector<char>& data, int32_t& width, int32_t& height) {
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
