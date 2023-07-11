#include "vulkan_material.h"

#include <ranges>
#include <utility>

#include "managers/texture_manager.h"

#include "renderer/backend/renderer.h"

#include "renderer/backend/vulkan/vulkan_context.h"
#include "renderer/backend/vulkan/vulkan_shader.h"
#include "renderer/backend/vulkan/vulkan_command_buffer.h"
#include "renderer/backend/vulkan/vulkan_texture.h"
#include "renderer/backend/vulkan/vulkan_image.h"
#include "renderer/backend/vulkan/vulkan_descriptors.h"
#include "renderer/backend/vulkan/vulkan_buffers.h"

namespace Phos {

constexpr uint32_t MATERIAL_DESCRIPTOR_SET = 2;

VulkanMaterial::VulkanMaterial(const std::shared_ptr<Shader>& shader, std::string name) : m_name(std::move(name)) {
    m_shader = std::dynamic_pointer_cast<VulkanShader>(shader);
    m_allocator = std::make_shared<VulkanDescriptorAllocator>();

    const auto descriptors = m_shader->descriptors_in_set(MATERIAL_DESCRIPTOR_SET);

    const auto white_texture =
        std::dynamic_pointer_cast<VulkanTexture>(Renderer::texture_manager()->get_white_texture());

    for (const auto& descriptor : descriptors) {
        if (descriptor.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
            m_textures.insert({descriptor.name, white_texture});
        }

        if (descriptor.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
            auto ubo = std::make_shared<VulkanUniformBuffer>(descriptor.size);

            const auto data = std::vector<float>(descriptor.size / sizeof(float), 1.0f);
            ubo->set_data(data.data());

            m_uniform_buffers.insert({descriptor.name, ubo});
        }

        m_name_to_descriptor_info.insert({descriptor.name, descriptor});
    }
}

void VulkanMaterial::set(const std::string& name, glm::vec3 data) {
    const auto data_size = sizeof(glm::vec3);

    const auto member = find_uniform_buffer_member(name);
    if (!member.has_value()) {
        PS_ERROR("Could not find descriptor with name: {}", name);
        return;
    }

    if (member->size != data_size) {
        PS_ERROR("Size of data with name {} does not match with input data ({}, {})", name, member->size, data_size);
        return;
    }

    const std::string ubo_name = name.substr(0, name.find('.'));

    const auto& d = m_uniform_buffers[ubo_name];
    d->set_data(&data, member->size, member->offset);
}

void VulkanMaterial::set(const std::string& name, glm::vec4 data) {
    const auto data_size = sizeof(glm::vec4);

    const auto member = find_uniform_buffer_member(name);
    if (!member.has_value()) {
        PS_ERROR("Could not find descriptor with name: {}", name);
        return;
    }

    if (member->size != data_size) {
        PS_ERROR("Size of data with name {} does not match with input data ({}, {})", name, member->size, data_size);
        return;
    }

    const std::string ubo_name = name.substr(0, name.find('.'));

    const auto& d = m_uniform_buffers[ubo_name];
    d->set_data(&data, member->size, member->offset);
}

void VulkanMaterial::set(const std::string& name, std::shared_ptr<Texture> texture) {
    if (!m_textures.contains(name)) {
        PS_ERROR("Material {} does not contain texture with name: {}", m_name, name);
        return;
    }

    m_textures[name] = std::dynamic_pointer_cast<VulkanTexture>(texture);
}

bool VulkanMaterial::bake() {
    auto builder = VulkanDescriptorBuilder::begin(VulkanContext::descriptor_layout_cache, m_allocator);

    // Build textures
    std::vector<VkDescriptorImageInfo*> image_infos;
    for (const auto& [name, texture] : m_textures) {
        const auto& image = std::dynamic_pointer_cast<VulkanImage>(texture->get_image());

        auto* info = new VkDescriptorImageInfo{};
        info->imageView = image->view();
        info->sampler = texture->sampler();
        info->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        image_infos.push_back(info);

        const auto& descriptor = m_name_to_descriptor_info[name];
        builder = builder.bind_image(
            descriptor.binding, *image_infos.back(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptor.stage);
    }

    // Build uniform buffers
    std::vector<VkDescriptorBufferInfo*> buffer_infos;
    for (const auto& [name, buffer] : m_uniform_buffers) {
        auto* info = new VkDescriptorBufferInfo{};
        info->buffer = buffer->handle();
        info->range = buffer->size();
        info->offset = 0;

        buffer_infos.push_back(info);

        const auto& descriptor = m_name_to_descriptor_info[name];
        builder = builder.bind_buffer(
            descriptor.binding, *buffer_infos.back(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptor.stage);
    }

    const auto success = builder.build(m_set);

    // Delete temporal infos
    for (auto* info : image_infos)
        delete info;
    for (auto* info : buffer_infos)
        delete info;

    return success;
}

void VulkanMaterial::bind(const std::shared_ptr<VulkanCommandBuffer>& command_buffer) const {
    vkCmdBindDescriptorSets(command_buffer->handle(),
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_shader->get_pipeline_layout(),
                            MATERIAL_DESCRIPTOR_SET,
                            1,
                            &m_set,
                            0,
                            nullptr);
}

std::optional<VulkanUniformBufferMember> VulkanMaterial::find_uniform_buffer_member(const std::string& name) {
    for (const auto& [_, info] : m_name_to_descriptor_info) {
        if (info.type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
            continue;

        auto member = std::ranges::find_if(info.members, [&](const VulkanUniformBufferMember& mem) {
            const std::string complete_name = fmt::format("{}.{}", info.name, mem.name);
            return complete_name == name;
        });

        if (member != info.members.end())
            return *member;
    }

    return {};
}

} // namespace Phos
