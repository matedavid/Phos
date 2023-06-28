#include "vulkan_material.h"

#include <ranges>

#include "renderer/backend/vulkan/vulkan_context.h"
#include "renderer/backend/vulkan/vulkan_shader.h"
#include "renderer/backend/vulkan/vulkan_command_buffer.h"
#include "renderer/backend/vulkan/vulkan_texture.h"
#include "renderer/backend/vulkan/vulkan_image.h"
#include "renderer/backend/vulkan/vulkan_descriptors.h"

namespace Phos {

VulkanMaterial::VulkanMaterial(const Definition& definition) : m_definition(definition) {
    m_shader = std::dynamic_pointer_cast<VulkanShader>(definition.shader);
    m_allocator = std::make_shared<VulkanDescriptorAllocator>();

    auto builder = VulkanDescriptorBuilder::begin(VulkanContext::descriptor_layout_cache, m_allocator);

    const auto descriptors = m_shader->descriptors_in_set(2);

    for (const auto& [name, texture] : definition.textures) {
        const auto result =
            std::ranges::find_if(descriptors, [&](const VulkanDescriptorInfo& info) { return info.name == name; });

        if (result == descriptors.end()) {
            PS_ERROR("Texture with name {} not found in shader", name)
            continue;
        }

        const auto& native_texture = std::dynamic_pointer_cast<VulkanTexture>(texture);
        const auto& native_image = std::dynamic_pointer_cast<VulkanImage>(texture->get_image());

        VkDescriptorImageInfo info{};
        info.imageView = native_image->view();
        info.sampler = native_texture->sampler();
        info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        builder = builder.bind_image(result->binding, info, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, result->stage);
    }

    PS_ASSERT(builder.build(m_set), "Failed to create {} Material descriptor set", definition.name)
}

void VulkanMaterial::bind(const std::shared_ptr<VulkanCommandBuffer>& command_buffer) const {
    vkCmdBindDescriptorSets(
        command_buffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_shader->get_pipeline_layout(), 2, 1, &m_set, 0, 0);
}

} // namespace Phos
