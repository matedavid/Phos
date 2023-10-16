#include "vulkan_cubemap.h"

#include <filesystem>
#include <stb_image.h>

#include "managers/shader_manager.h"

#include "renderer/backend/renderer.h"
#include "renderer/backend/vulkan/vulkan_context.h"
#include "renderer/backend/vulkan/vulkan_device.h"
#include "renderer/backend/vulkan/vulkan_command_buffer.h"
#include "renderer/backend/vulkan/vulkan_image.h"
#include "renderer/backend/vulkan/vulkan_texture.h"
#include "renderer/backend/vulkan/vulkan_buffer.h"
#include "renderer/backend/vulkan/vulkan_compute_pipeline.h"

namespace Phos {

VulkanCubemap::VulkanCubemap(const Faces& sides) {
    init(sides);
}

VulkanCubemap::VulkanCubemap(const Faces& faces, const std::string& directory) {
    const auto dir = std::filesystem::path(directory);

    const auto directory_sides = Cubemap::Faces{
        .right = dir / faces.right,
        .left = dir / faces.left,
        .top = dir / faces.top,
        .bottom = dir / faces.bottom,
        .front = dir / faces.front,
        .back = dir / faces.back,
    };

    init(directory_sides);
}

VulkanCubemap::VulkanCubemap(const std::string& equirectangular_path) {
    const auto equirectangular_texture = Texture::create(equirectangular_path);

    constexpr uint32_t cubemap_size = 512;
    m_image = std::make_shared<VulkanImage>(VulkanImage::Description{
        .width = cubemap_size,
        .height = cubemap_size,
        .type = VulkanImage::Type::Cubemap,
        .format = VulkanImage::Format::R32G32B32A32_SFLOAT,
        .num_layers = 6,
        .storage = true,
    });

    auto pipeline = VulkanComputePipeline({
        .shader = Renderer::shader_manager()->get_builtin_shader("EquirectangularToCubemap"),
    });

    // @TODO: Doing because Texture creation sets layout to SHADER_READ_ONLY_OPTIMAL, but compute_pipeline set
    // expects GENERAL, or READ_ONLY if attachment flag set, which is not the case.
    const auto native_eq_image = std::dynamic_pointer_cast<VulkanImage>(equirectangular_texture->get_image());
    native_eq_image->transition_layout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);

    VulkanCommandBuffer::submit_single_time(VulkanQueue::Type::Compute, [&](const auto& cb) {
        pipeline.add_step(
            [&](ComputePipeline::StepBuilder& builder) {
                const auto output_texture = Texture::create(m_image);

                builder.set("uOutput", output_texture);
                builder.set("uEquirectangularImage", equirectangular_texture);
            },
            {cubemap_size / 16, cubemap_size / 16, 8});

        pipeline.execute(cb);
    });

    m_image->transition_layout(VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

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

VulkanCubemap::~VulkanCubemap() {
    vkDestroySampler(VulkanContext::device->handle(), m_sampler, nullptr);
}

void VulkanCubemap::init(const Faces& faces) {
    int32_t width, height;

    // @NOTE: At the moment, only works if all images in cubemap have the same dimensions
    std::vector<unsigned char> data;

    // Order from:
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap16.html#_cube_map_face_selection_and_transformations
    load_face(faces.right, data, width, height, 0);
    load_face(faces.left, data, width, height, 1);
    load_face(faces.top, data, width, height, 2);
    load_face(faces.bottom, data, width, height, 3);
    load_face(faces.front, data, width, height, 4);
    load_face(faces.back, data, width, height, 5);

    const auto image_size = static_cast<uint32_t>(width * height * 4) * 6;

    m_image = std::make_shared<VulkanImage>(VulkanImage::Description{
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

void VulkanCubemap::load_face(const std::string& path,
                              std::vector<unsigned char>& data,
                              int32_t& width,
                              int32_t& height,
                              uint32_t idx) {
    int32_t width_l, height_l, channels;
    stbi_uc* pixels = stbi_load(path.c_str(), &width_l, &height_l, &channels, STBI_rgb_alpha);

    if (idx == 0) {
        const auto data_size = static_cast<uint32_t>((width_l * height_l * 4) * 6);
        data.resize(data_size);

        width = width_l;
        height = height_l;
    } else {
        PS_ASSERT(width_l == width && height_l == height,
                  "Size of cubemap image with idx {} does not match expected size ({} != {}, {} != {})",
                  idx,
                  width_l,
                  width,
                  height_l,
                  height)
    }

    const auto image_size = static_cast<uint32_t>(width * height * 4);
    memcpy(data.data() + idx * image_size, pixels, image_size);

    stbi_image_free(pixels);
}

std::shared_ptr<Image> VulkanCubemap::get_image() const {
    return std::dynamic_pointer_cast<Image>(m_image);
}

VkImageView VulkanCubemap::view() const {
    return m_image->view();
}

VkSampler VulkanCubemap::sampler() const {
    return m_sampler;
}

} // namespace Phos
