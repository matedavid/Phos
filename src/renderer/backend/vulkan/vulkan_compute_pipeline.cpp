#include "vulkan_compute_pipeline.h"

#include <fstream>

#include "renderer/backend/vulkan/vulkan_context.h"
#include "renderer/backend/vulkan/vulkan_device.h"
#include "renderer/backend/vulkan/vulkan_command_buffer.h"
#include "renderer/backend/vulkan/vulkan_texture.h"
#include "renderer/backend/vulkan/vulkan_image.h"
#include "renderer/backend/vulkan/vulkan_descriptors.h"
#include "renderer/backend/vulkan/vulkan_shader.h"

namespace Phos {

VulkanComputePipeline::VulkanComputePipeline(const Description& description) {
    m_shader = std::dynamic_pointer_cast<VulkanShader>(description.shader);

    VkComputePipelineCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    create_info.stage = m_shader->get_shader_stage_create_infos()[0];
    create_info.layout = m_shader->get_pipeline_layout();

    VK_CHECK(vkCreateComputePipelines(VulkanContext::device->handle(), nullptr, 1, &create_info, nullptr, &m_pipeline))

    // Allocator for descriptor builder
    m_allocator = std::make_shared<VulkanDescriptorAllocator>();
}

VulkanComputePipeline::~VulkanComputePipeline() {
    vkDestroyPipeline(VulkanContext::device->handle(), m_pipeline, nullptr);
}

void VulkanComputePipeline::bind(const std::shared_ptr<CommandBuffer>& command_buffer) {
    const auto native_cb = std::dynamic_pointer_cast<VulkanCommandBuffer>(command_buffer);

    vkCmdBindPipeline(native_cb->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);
    vkCmdBindDescriptorSets(native_cb->handle(),
                            VK_PIPELINE_BIND_POINT_COMPUTE,
                            m_shader->get_pipeline_layout(),
                            0,
                            1,
                            &m_descriptor_sets[m_descriptor_sets.size() - 1],
                            0,
                            nullptr);
}

void VulkanComputePipeline::bind_push_constants(const std::shared_ptr<CommandBuffer>& command_buffer,
                                                std::string_view name,
                                                uint32_t size,
                                                const void* data) {
    const auto& native_command_buffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(command_buffer);

    // @TODO: Should check if push constant exists in Pipeline, using name

    vkCmdPushConstants(
        native_command_buffer->handle(), m_shader->get_pipeline_layout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, size, data);
}

void VulkanComputePipeline::execute(const std::shared_ptr<CommandBuffer>& command_buffer, glm::ivec3 work_groups) {
    const auto native_cb = std::dynamic_pointer_cast<VulkanCommandBuffer>(command_buffer);
    vkCmdDispatch(native_cb->handle(), work_groups.x, work_groups.y, work_groups.z);
}

bool VulkanComputePipeline::bake() {
    auto builder = VulkanDescriptorBuilder::begin(VulkanContext::descriptor_layout_cache, m_allocator);

    for (const auto& [info, write] : m_buffer_descriptor_info) {
        builder = builder.bind_buffer(info.binding, write, info.type, info.stage);
    }

    for (const auto& [info, write] : m_image_descriptor_info) {
        builder = builder.bind_image(info.binding, write, info.type, info.stage);
    }

    m_buffer_descriptor_info.clear();
    m_image_descriptor_info.clear();

    VkDescriptorSet set;
    const bool built = builder.build(set);
    m_descriptor_sets.push_back(set);

    return built;
}

void VulkanComputePipeline::invalidate() {
    m_buffer_descriptor_info.clear();
    m_image_descriptor_info.clear();

    m_allocator = std::make_shared<VulkanDescriptorAllocator>();
    m_descriptor_sets.clear();
}

void VulkanComputePipeline::set(std::string_view name, const std::shared_ptr<Texture>& texture) {
    const auto native_texture = std::dynamic_pointer_cast<VulkanTexture>(texture);
    const auto native_image = std::dynamic_pointer_cast<VulkanImage>(texture->get_image());

    const auto info = m_shader->descriptor_info(name);
    PS_ASSERT(info.has_value() && is_valid_texture_type(info->type),
              "Compute Pipeline does not contain texture with name: {}",
              name)

    VkDescriptorImageInfo descriptor{};
    descriptor.imageView = native_image->view();
    descriptor.sampler = native_texture->sampler();
    descriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL; // @TODO

    m_image_descriptor_info.emplace_back(info.value(), descriptor);
}

void VulkanComputePipeline::set(std::string_view name, const std::shared_ptr<Texture>& texture, uint32_t mip_level) {
    const auto native_texture = std::dynamic_pointer_cast<VulkanTexture>(texture);
    const auto native_image = std::dynamic_pointer_cast<VulkanImage>(texture->get_image());

    const auto info = m_shader->descriptor_info(name);
    PS_ASSERT(info.has_value() && is_valid_texture_type(info->type),
              "Compute Pipeline does not contain texture with name: {}",
              name)

    VkDescriptorImageInfo descriptor{};
    descriptor.imageView = native_image->mip_view(mip_level);
    descriptor.sampler = native_texture->sampler();
    descriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL; // @TODO

    m_image_descriptor_info.emplace_back(info.value(), descriptor);
}

bool VulkanComputePipeline::is_valid_texture_type(VkDescriptorType type) {
    return type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER || type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
}

VkPipelineLayout VulkanComputePipeline::layout() const {
    return m_shader->get_pipeline_layout();
}

} // namespace Phos
