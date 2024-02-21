#include "vulkan_compute_pipeline.h"

#include <cstring>

#include "vk_core.h"

#include "utility/logging.h"

#include "renderer/backend/renderer.h"

#include "renderer/backend/vulkan/vulkan_context.h"
#include "renderer/backend/vulkan/vulkan_device.h"
#include "renderer/backend/vulkan/vulkan_command_buffer.h"
#include "renderer/backend/vulkan/vulkan_texture.h"
#include "renderer/backend/vulkan/vulkan_image.h"
#include "renderer/backend/vulkan/vulkan_descriptors.h"
#include "renderer/backend/vulkan/vulkan_shader.h"

namespace Phos {

static bool is_valid_texture_type(VkDescriptorType type) {
    return type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER || type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
}

//
// VulkanComputePipelineStepBuilder
//

VulkanComputePipelineStepBuilder::VulkanComputePipelineStepBuilder(std::shared_ptr<VulkanShader> shader,
                                                                   std::shared_ptr<VulkanDescriptorAllocator> allocator)
      : m_shader(std::move(shader)), m_allocator(std::move(allocator)) {}

void VulkanComputePipelineStepBuilder::set_push_constants(std::string_view name, uint32_t size, const void* data) {
    const auto info = m_shader->push_constant_info(name);
    PHOS_ASSERT(info.has_value(), "Compute Pipeline does not contain push constant with name: {}", name);
    PHOS_ASSERT(
        info->size == size, "Push constant with name: {} has incorrect size ({} != {})", name, size, info->size);

    std::vector<unsigned char> vec_data(size);
    memcpy(vec_data.data(), data, size);

    m_push_constants_info.push_back(vec_data);
}

void VulkanComputePipelineStepBuilder::set(std::string_view name, const std::shared_ptr<Texture>& texture) {
    const auto native_texture = std::dynamic_pointer_cast<VulkanTexture>(texture);
    const auto native_image = std::dynamic_pointer_cast<VulkanImage>(texture->get_image());

    const auto info = m_shader->descriptor_info(name);
    PHOS_ASSERT(info.has_value() && is_valid_texture_type(info->type),
                "Compute Pipeline does not contain texture with name: {}",
                name);

    VkDescriptorImageInfo descriptor{};
    descriptor.imageView = native_image->view();
    descriptor.sampler = native_texture->sampler();
    descriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL; // @TODO

    // @TODO: UGLY :)
    if (native_image->description().attachment)
        descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    m_image_descriptor_info.emplace_back(info.value(), descriptor);
}

void VulkanComputePipelineStepBuilder::set(std::string_view name,
                                           const std::shared_ptr<Texture>& texture,
                                           uint32_t mip_level) {
    const auto native_texture = std::dynamic_pointer_cast<VulkanTexture>(texture);
    const auto native_image = std::dynamic_pointer_cast<VulkanImage>(texture->get_image());

    const auto info = m_shader->descriptor_info(name);
    PHOS_ASSERT(info.has_value() && is_valid_texture_type(info->type),
                "Compute Pipeline does not contain texture with name: {}",
                name);

    VkDescriptorImageInfo descriptor{};
    descriptor.imageView = native_image->mip_view(mip_level);
    descriptor.sampler = native_texture->sampler();
    descriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL; // @TODO

    m_image_descriptor_info.emplace_back(info.value(), descriptor);
}

VkDescriptorSet VulkanComputePipelineStepBuilder::build() const {
    auto builder = VulkanDescriptorBuilder::begin(VulkanContext::descriptor_layout_cache, m_allocator);

    for (const auto& [info, write] : m_buffer_descriptor_info) {
        builder = builder.bind_buffer(info.binding, &write, info.type, info.stage);
    }

    for (const auto& [info, write] : m_image_descriptor_info) {
        builder = builder.bind_image(info.binding, &write, info.type, info.stage);
    }

    VkDescriptorSet set;
    [[maybe_unused]] const auto built = builder.build(set);
    PHOS_ASSERT(built, "Could not build ComputePipelineStepBuilder descriptor set");

    return set;
}

std::vector<unsigned char> VulkanComputePipelineStepBuilder::push_constants() const {
    if (m_push_constants_info.empty())
        return {};
    else
        return m_push_constants_info[0];
}

//
// VulkanComputePipeline
//

VulkanComputePipeline::VulkanComputePipeline(const Description& description) {
    m_shader = std::dynamic_pointer_cast<VulkanShader>(description.shader);

    VkComputePipelineCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    create_info.stage = m_shader->get_shader_stage_create_infos()[0];
    create_info.layout = m_shader->get_pipeline_layout();

    VK_CHECK(vkCreateComputePipelines(VulkanContext::device->handle(), nullptr, 1, &create_info, nullptr, &m_pipeline));

    // Allocator for descriptor builder
    m_allocator = std::make_shared<VulkanDescriptorAllocator>();
}

VulkanComputePipeline::~VulkanComputePipeline() {
    vkDestroyPipeline(VulkanContext::device->handle(), m_pipeline, nullptr);
}

void VulkanComputePipeline::add_step(const std::function<void(StepBuilder&)>& func, glm::uvec3 work_groups) {
    const auto builder = new VulkanComputePipelineStepBuilder(m_shader, m_allocator);

    func(*builder);

    auto step = Step{};
    step.set = builder->build();
    step.push_constant = builder->push_constants();
    step.work_groups = work_groups;

    m_steps.push_back(step);

    delete builder;
}

void VulkanComputePipeline::execute(const std::shared_ptr<CommandBuffer>& command_buffer) {
    const auto native_cb = std::dynamic_pointer_cast<VulkanCommandBuffer>(command_buffer);

    vkCmdBindPipeline(native_cb->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);

    for (const auto& step : m_steps) {
        // Set push constants
        if (!step.push_constant.empty()) {
            vkCmdPushConstants(native_cb->handle(),
                               m_shader->get_pipeline_layout(),
                               VK_SHADER_STAGE_COMPUTE_BIT,
                               0,
                               static_cast<uint32_t>(step.push_constant.size()),
                               step.push_constant.data());
        }

        // Set descriptor set
        vkCmdBindDescriptorSets(native_cb->handle(),
                                VK_PIPELINE_BIND_POINT_COMPUTE,
                                m_shader->get_pipeline_layout(),
                                0,
                                1,
                                &step.set,
                                0,
                                nullptr);

        // Dispatch
        vkCmdDispatch(native_cb->handle(), step.work_groups.x, step.work_groups.y, step.work_groups.z);
    }
}

} // namespace Phos
