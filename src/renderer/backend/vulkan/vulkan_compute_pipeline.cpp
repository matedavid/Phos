#include "vulkan_compute_pipeline.h"

#include <fstream>

#include "renderer/backend/vulkan/vulkan_context.h"
#include "renderer/backend/vulkan/vulkan_device.h"
#include "renderer/backend/vulkan/vulkan_command_buffer.h"

namespace Phos {

VulkanComputePipeline::VulkanComputePipeline() {
    std::ifstream file("../shaders/build/Bloom.Compute.spv", std::ios::ate | std::ios::binary);
    PS_ASSERT(file.is_open(), "Could not open compute shader file")

    const auto size = (uint32_t)file.tellg();
    std::vector<char> content(size);

    file.seekg(0);
    file.read(content.data(), size);

    file.close();

    VkShaderModuleCreateInfo shader_create_info{};
    shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_create_info.codeSize = size;
    shader_create_info.pCode = reinterpret_cast<const uint32_t*>(content.data());

    VK_CHECK(vkCreateShaderModule(VulkanContext::device->handle(), &shader_create_info, nullptr, &m_shader))

    VkPipelineShaderStageCreateInfo shader_stage_create_info{};
    shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage_create_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shader_stage_create_info.module = m_shader;
    shader_stage_create_info.pName = "main";

    const std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        },
        {
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        },
    };

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
    descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_create_info.bindingCount = (uint32_t)bindings.size();
    descriptor_set_layout_create_info.pBindings = bindings.data();

    VK_CHECK(vkCreateDescriptorSetLayout(
        VulkanContext::device->handle(), &descriptor_set_layout_create_info, nullptr, &m_descriptor_set_layout))

    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.setLayoutCount = 1;
    pipeline_layout_create_info.pSetLayouts = &m_descriptor_set_layout;
    pipeline_layout_create_info.pushConstantRangeCount = 0;

    VK_CHECK(vkCreatePipelineLayout(VulkanContext::device->handle(), &pipeline_layout_create_info, nullptr, &m_layout))

    VkComputePipelineCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    create_info.stage = shader_stage_create_info;
    create_info.layout = m_layout;

    VK_CHECK(vkCreateComputePipelines(VulkanContext::device->handle(), nullptr, 1, &create_info, nullptr, &m_pipeline))
}

VulkanComputePipeline::~VulkanComputePipeline() {
    vkDestroyPipeline(VulkanContext::device->handle(), m_pipeline, nullptr);
    vkDestroyPipelineLayout(VulkanContext::device->handle(), m_layout, nullptr);
    vkDestroyDescriptorSetLayout(VulkanContext::device->handle(), m_descriptor_set_layout, nullptr);
    vkDestroyShaderModule(VulkanContext::device->handle(), m_shader, nullptr);
}

void VulkanComputePipeline::bind(const std::shared_ptr<CommandBuffer>& command_buffer) {
    const auto native_cb = std::dynamic_pointer_cast<VulkanCommandBuffer>(command_buffer);
    vkCmdBindPipeline(native_cb->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);
}

void VulkanComputePipeline::execute(const std::shared_ptr<CommandBuffer>& command_buffer, glm::ivec3 work_groups) {
    const auto native_cb = std::dynamic_pointer_cast<VulkanCommandBuffer>(command_buffer);
    vkCmdDispatch(native_cb->handle(), work_groups.x, work_groups.y, work_groups.z);
}

} // namespace Phos
