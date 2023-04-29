#include "vulkan_graphics_pipeline.h"

#include <ranges>

#include "renderer/vulkan_device.h"
#include "renderer/vulkan_shader_module.h"
#include "renderer/vulkan_render_pass.h"
#include "renderer/vulkan_command_buffer.h"
#include "renderer/vulkan_context.h"

VulkanGraphicsPipeline::VulkanGraphicsPipeline(const Description& description) {
    // Shaders
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    for (const auto& shader : description.shader_modules) {
        shader_stages.push_back(shader->get_shader_stage_create_info());
    }

    // Vertex input
    const auto& iterator = std::ranges::find_if(
        description.shader_modules, [](const auto& s) { return s->get_stage() == VulkanShaderModule::Stage::Vertex; });

    CORE_ASSERT(iterator != description.shader_modules.end(), "Pipeline must contain vertex shader");
    const auto& vertex_shader = iterator->get();

    const auto binding_description = vertex_shader->get_binding_description();
    const auto attribute_descriptions = vertex_shader->get_attribute_descriptions();

    VkPipelineVertexInputStateCreateInfo vertex_input_create_info{};
    vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_create_info.vertexBindingDescriptionCount = static_cast<bool>(binding_description.has_value());
    vertex_input_create_info.pVertexBindingDescriptions =
        binding_description.has_value() ? &binding_description.value() : VK_NULL_HANDLE;
    vertex_input_create_info.vertexAttributeDescriptionCount = (uint32_t)attribute_descriptions.size();
    vertex_input_create_info.pVertexAttributeDescriptions =
        !attribute_descriptions.empty() ? attribute_descriptions.data() : VK_NULL_HANDLE;

    // Input Assembly
    VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info{};
    input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // TODO: Make this configurable?
    input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

    // Tessellation State (Not using)
    VkPipelineTessellationStateCreateInfo tessellation_create_info{};
    tessellation_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;

    // Viewport State (Using dynamic state for Viewport and Scissor)
    VkPipelineViewportStateCreateInfo viewport_create_info{};
    viewport_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_create_info.viewportCount = 1;
    viewport_create_info.scissorCount = 1;

    // Rasterization State
    // TODO: Make rasterization stage configurable?
    VkPipelineRasterizationStateCreateInfo rasterization_create_info{};
    rasterization_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_create_info.depthClampEnable = VK_FALSE;
    rasterization_create_info.rasterizerDiscardEnable = VK_FALSE;
    rasterization_create_info.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterization_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterization_create_info.depthBiasEnable = VK_FALSE;
    rasterization_create_info.lineWidth = 1.0f;

    // Multisample State
    VkPipelineMultisampleStateCreateInfo multisample_create_info{};
    multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_create_info.sampleShadingEnable = VK_FALSE;
    multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Depth-Stencil State
    VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info{};
    depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_create_info.depthTestEnable = VK_TRUE;
    depth_stencil_create_info.depthWriteEnable = VK_FALSE;
    depth_stencil_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_create_info.stencilTestEnable = VK_FALSE;
    depth_stencil_create_info.minDepthBounds = 0.0f;
    depth_stencil_create_info.maxDepthBounds = 1.0f;

    // Color Blend State
    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.blendEnable = VK_FALSE;
    color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo color_blend_create_info{};
    color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_create_info.logicOpEnable = VK_FALSE;
    color_blend_create_info.attachmentCount = 1;
    color_blend_create_info.pAttachments = &color_blend_attachment;

    // Dynamic State
    constexpr std::array<VkDynamicState, 2> dynamic_state = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
    dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_create_info.dynamicStateCount = dynamic_state.size();
    dynamic_state_create_info.pDynamicStates = dynamic_state.data();

    // Pipeline Layout

    // TODO: At this moment descriptor sets are probably separated by shader module (vertex, fragment), leaving like
    // this at the moment but probably not the most efficient way to organize descriptor sets

    std::vector<VkDescriptorSetLayout> descriptor_sets_layout;
    for (const auto& shader : description.shader_modules) {
        for (const auto& set_layout : shader->get_descriptor_sets_layout()) {
            descriptor_sets_layout.push_back(set_layout);
        }
    }

    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.setLayoutCount = (uint32_t)descriptor_sets_layout.size();
    pipeline_layout_create_info.pSetLayouts = descriptor_sets_layout.data();
    pipeline_layout_create_info.pushConstantRangeCount = 0;

    VK_CHECK(vkCreatePipelineLayout(
        VulkanContext::device->handle(), &pipeline_layout_create_info, nullptr, &m_pipeline_layout))

    //
    // Create pipeline
    //
    VkGraphicsPipelineCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    create_info.stageCount = (uint32_t)shader_stages.size();
    create_info.pStages = shader_stages.data();
    create_info.pVertexInputState = &vertex_input_create_info;
    create_info.pInputAssemblyState = &input_assembly_create_info;
    create_info.pTessellationState = &tessellation_create_info;
    create_info.pViewportState = &viewport_create_info;
    create_info.pRasterizationState = &rasterization_create_info;
    create_info.pMultisampleState = &multisample_create_info;
    create_info.pDepthStencilState = &depth_stencil_create_info;
    create_info.pColorBlendState = &color_blend_create_info;
    create_info.pDynamicState = &dynamic_state_create_info;
    create_info.layout = m_pipeline_layout;
    create_info.renderPass = description.render_pass->handle();
    create_info.subpass = 0;

    VK_CHECK(vkCreateGraphicsPipelines(VulkanContext::device->handle(), nullptr, 1, &create_info, nullptr, &m_pipeline))
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline() {
    // Destroy pipeline layout
    vkDestroyPipelineLayout(VulkanContext::device->handle(), m_pipeline_layout, nullptr);

    // Destroy pipeline
    vkDestroyPipeline(VulkanContext::device->handle(), m_pipeline, nullptr);
}

void VulkanGraphicsPipeline::bind(const std::shared_ptr<VulkanCommandBuffer>& command_buffer) const {
    vkCmdBindPipeline(command_buffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
}
