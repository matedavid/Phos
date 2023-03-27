#include "vulkan_graphics_pipeline.h"

#include <ranges>

#include "renderer/vulkan_device.h"
#include "renderer/vulkan_shader_module.h"

VulkanGraphicsPipeline::VulkanGraphicsPipeline(std::shared_ptr<VulkanDevice> device, const Description& description)
    : m_device(std::move(device)) {
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
    const auto attribute_description = vertex_shader->get_attribute_descriptions();

    VkPipelineVertexInputStateCreateInfo vertex_input_create_info{};
    vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_create_info.vertexBindingDescriptionCount = 1;
    vertex_input_create_info.pVertexBindingDescriptions = &binding_description;
    vertex_input_create_info.vertexAttributeDescriptionCount = (uint32_t)attribute_description.size();
    vertex_input_create_info.pVertexAttributeDescriptions = attribute_description.data();

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
    for (const auto& shader : description.shader_modules) {
        for (const auto& set_layout_create_info : shader->get_descriptor_sets()) {
            VkDescriptorSetLayout set_layout;
            VK_CHECK(vkCreateDescriptorSetLayout(m_device->handle(), &set_layout_create_info, nullptr, &set_layout));

            m_descriptor_set_layouts.push_back(set_layout);
        }
    }

    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.setLayoutCount = (uint32_t)m_descriptor_set_layouts.size();
    pipeline_layout_create_info.pSetLayouts = m_descriptor_set_layouts.data();
    pipeline_layout_create_info.pushConstantRangeCount = 0;

    VK_CHECK(vkCreatePipelineLayout(m_device->handle(), &pipeline_layout_create_info, nullptr, &m_pipeline_layout))

    // Render pass
    // TODO: This should definitely be configurable

    VkAttachmentDescription attachment_description{};
    attachment_description.format = VK_FORMAT_R8G8B8A8_SRGB;
    attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment{};
    color_attachment.attachment = 0;
    color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass_description{};
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description.colorAttachmentCount = 1;
    subpass_description.pColorAttachments = &color_attachment;

    VkSubpassDependency subpass_dependency{};
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.srcAccessMask = 0;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_create_info{};
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount = 1;
    render_pass_create_info.pAttachments = &attachment_description;
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass_description;
    render_pass_create_info.dependencyCount = 1;
    render_pass_create_info.pDependencies = &subpass_dependency;

    VK_CHECK(vkCreateRenderPass(m_device->handle(), &render_pass_create_info, nullptr, &m_render_pass))

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
    create_info.renderPass = m_render_pass;
    create_info.subpass = 0;

    VK_CHECK(vkCreateGraphicsPipelines(m_device->handle(), nullptr, 1, &create_info, nullptr, &m_pipeline))
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline() {
    // Destroy descriptor sets
    for (const auto& descriptor_set_layout : m_descriptor_set_layouts) {
        vkDestroyDescriptorSetLayout(m_device->handle(), descriptor_set_layout, nullptr);
    }

    // Destroy pipeline layout
    vkDestroyPipelineLayout(m_device->handle(), m_pipeline_layout, nullptr);

    // Destroy render pass
    vkDestroyRenderPass(m_device->handle(), m_render_pass, nullptr);

    // Destroy pipeline
    vkDestroyPipeline(m_device->handle(), m_pipeline, nullptr);
}
