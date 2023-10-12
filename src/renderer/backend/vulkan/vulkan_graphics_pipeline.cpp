#include "vulkan_graphics_pipeline.h"

#include <ranges>

#include "renderer/backend/vulkan/vulkan_shader.h"
#include "renderer/backend/vulkan/vulkan_command_buffer.h"
#include "renderer/backend/vulkan/vulkan_context.h"
#include "renderer/backend/vulkan/vulkan_framebuffer.h"
#include "renderer/backend/vulkan/vulkan_buffers.h"
#include "renderer/backend/vulkan/vulkan_image.h"
#include "renderer/backend/vulkan/vulkan_texture.h"
#include "renderer/backend/vulkan/vulkan_descriptors.h"
#include "renderer/backend/vulkan/vulkan_cubemap.h"

namespace Phos {

VulkanGraphicsPipeline::VulkanGraphicsPipeline(const Description& description) {
    // TODO: Maybe do differently?
    m_shader = std::dynamic_pointer_cast<VulkanShader>(description.shader);
    m_target_framebuffer = description.target_framebuffer;
    const auto target_framebuffer = std::dynamic_pointer_cast<VulkanFramebuffer>(description.target_framebuffer);

    // Shaders
    const std::vector<VkPipelineShaderStageCreateInfo> shader_stages = m_shader->get_shader_stage_create_infos();

    // Vertex input
    const auto binding_description = m_shader->get_binding_description();
    const auto attribute_descriptions = m_shader->get_attribute_descriptions();

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
    VkPipelineRasterizationStateCreateInfo rasterization_create_info{};
    rasterization_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_create_info.depthClampEnable = VK_FALSE;
    rasterization_create_info.rasterizerDiscardEnable = VK_FALSE;
    rasterization_create_info.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterization_create_info.frontFace = get_rasterization_front_face(description.front_face);

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
    depth_stencil_create_info.depthWriteEnable = description.depth_write ? VK_TRUE : VK_FALSE;
    depth_stencil_create_info.depthCompareOp = get_depth_compare_op(description.depth_compare_op);
    depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_create_info.stencilTestEnable = VK_FALSE;
    depth_stencil_create_info.minDepthBounds = 0.0f;
    depth_stencil_create_info.maxDepthBounds = 1.0f;

    // Color Blend State
    std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments;

    for (const auto& attachment : description.target_framebuffer->get_attachments()) {
        if (VulkanImage::is_depth_format(attachment.image->format()))
            continue;

        VkPipelineColorBlendAttachmentState state{};
        state.blendEnable = VK_FALSE;
        state.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        color_blend_attachments.push_back(state);
    }

    VkPipelineColorBlendStateCreateInfo color_blend_create_info{};
    color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_create_info.logicOpEnable = VK_FALSE;
    color_blend_create_info.attachmentCount = static_cast<uint32_t>(color_blend_attachments.size());
    color_blend_create_info.pAttachments = color_blend_attachments.data();

    // Dynamic State
    constexpr std::array<VkDynamicState, 2> dynamic_state = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
    dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_create_info.dynamicStateCount = dynamic_state.size();
    dynamic_state_create_info.pDynamicStates = dynamic_state.data();

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
    create_info.layout = m_shader->get_pipeline_layout();
    create_info.renderPass = target_framebuffer->get_render_pass();
    create_info.subpass = 0;

    VK_CHECK(vkCreateGraphicsPipelines(VulkanContext::device->handle(), nullptr, 1, &create_info, nullptr, &m_pipeline))

    // Start descriptor builder
    m_allocator = std::make_shared<VulkanDescriptorAllocator>();
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline() {
    // Destroy pipeline
    vkDestroyPipeline(VulkanContext::device->handle(), m_pipeline, nullptr);
}

void VulkanGraphicsPipeline::bind(const std::shared_ptr<CommandBuffer>& command_buffer) {
    bool has_descriptor_set = !m_buffer_descriptor_info.empty() || !m_image_descriptor_info.empty();
    if (has_descriptor_set && m_set == VK_NULL_HANDLE) {
        PS_ASSERT(bake(), "Error when baking graphics pipeline")
    }

    const auto native_command_buffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(command_buffer);

    // Bind pipeline
    vkCmdBindPipeline(native_command_buffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

    // Bind descriptor set
    if (has_descriptor_set) {
        const std::vector<VkDescriptorSet> descriptor_sets = {m_set};
        vkCmdBindDescriptorSets(native_command_buffer->handle(),
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_shader->get_pipeline_layout(),
                                1, // Set = 1 for pipeline specific descriptors
                                static_cast<uint32_t>(descriptor_sets.size()),
                                descriptor_sets.data(),
                                0,
                                nullptr);
    }
}

VkPipelineLayout VulkanGraphicsPipeline::layout() const {
    return m_shader->get_pipeline_layout();
}

bool VulkanGraphicsPipeline::bake() {
    auto builder = VulkanDescriptorBuilder::begin(VulkanContext::descriptor_layout_cache, m_allocator);

    for (const auto& [info, write] : m_buffer_descriptor_info) {
        builder = builder.bind_buffer(info.binding, write, info.type, info.stage);
    }

    for (const auto& [info, write] : m_image_descriptor_info) {
        builder = builder.bind_image(info.binding, write, info.type, info.stage);
    }

    return builder.build(m_set);
}

std::shared_ptr<Framebuffer> VulkanGraphicsPipeline::target_framebuffer() const {
    return m_target_framebuffer;
}

void VulkanGraphicsPipeline::bind_push_constants(const std::shared_ptr<CommandBuffer>& command_buffer,
                                                 std::string_view name,
                                                 uint32_t size,
                                                 const void* data) {
    const auto& native_cb = std::dynamic_pointer_cast<VulkanCommandBuffer>(command_buffer);

    const auto info = m_shader->push_constant_info(name);
    PS_ASSERT(info.has_value(), "GraphicsPipeline does not contain push constant with name: {}", name)
    PS_ASSERT(info->size == size, "Push constant with name: {} has incorrect size ({} != {})", name, size, info->size)

    vkCmdPushConstants(native_cb->handle(), m_shader->get_pipeline_layout(), info->stage, 0, size, data);
}

void VulkanGraphicsPipeline::add_input(std::string_view name, const std::shared_ptr<UniformBuffer>& ubo) {
    const auto native_ubo = std::dynamic_pointer_cast<VulkanUniformBuffer>(ubo);

    const auto info = m_shader->descriptor_info(name);
    PS_ASSERT(info.has_value() && info->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
              "Graphics Pipeline does not contain Uniform Buffer with name: {}",
              name)

    VkDescriptorBufferInfo descriptor{};
    descriptor.buffer = native_ubo->handle();
    descriptor.range = native_ubo->size();
    descriptor.offset = 0;

    m_buffer_descriptor_info.emplace_back(info.value(), descriptor);
}

void VulkanGraphicsPipeline::add_input(std::string_view name, const std::shared_ptr<Texture>& texture) {
    const auto native_texture = std::dynamic_pointer_cast<VulkanTexture>(texture);
    const auto native_image = std::dynamic_pointer_cast<VulkanImage>(texture->get_image());

    const auto info = m_shader->descriptor_info(name);
    PS_ASSERT(info.has_value() && info->type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
              "Graphics Pipeline does not contain Sampler with name: {}",
              name)

    VkDescriptorImageInfo descriptor{};
    descriptor.imageView = native_image->view();
    descriptor.sampler = native_texture->sampler();
    descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // @TODO: UGLY :)
    if (native_image->description().storage && !native_image->description().attachment)
        descriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    m_image_descriptor_info.emplace_back(info.value(), descriptor);
}

void VulkanGraphicsPipeline::add_input(std::string_view name, const std::shared_ptr<Cubemap>& cubemap) {
    const auto native_cubemap = std::dynamic_pointer_cast<VulkanCubemap>(cubemap);

    const auto info = m_shader->descriptor_info(name);
    PS_ASSERT(info.has_value() && info->type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
              "Graphics Pipeline does not contain Sampler with name: {}",
              name)

    VkDescriptorImageInfo descriptor{};
    descriptor.imageView = native_cubemap->view();
    descriptor.sampler = native_cubemap->sampler();
    descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    m_image_descriptor_info.emplace_back(info.value(), descriptor);
}

VkCompareOp VulkanGraphicsPipeline::get_depth_compare_op(DepthCompareOp op) {
    switch (op) {
    default:
    case DepthCompareOp::Less:
        return VK_COMPARE_OP_LESS;
    case DepthCompareOp::LessEq:
        return VK_COMPARE_OP_LESS_OR_EQUAL;
    }
}

VkFrontFace VulkanGraphicsPipeline::get_rasterization_front_face(FrontFace face) {
    switch (face) {
    default:
    case FrontFace::Clockwise:
        return VK_FRONT_FACE_CLOCKWISE;
    case FrontFace::CounterClockwise:
        return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }
}

} // namespace Phos
