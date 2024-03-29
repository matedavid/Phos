#include "vulkan_renderer.h"

#include "vk_core.h"

#include "utility/profiling.h"

#include "renderer/mesh.h"
#include "renderer/camera.h"
#include "renderer/light.h"

#include "renderer/backend/vulkan/vulkan_context.h"
#include "renderer/backend/vulkan/vulkan_renderer_api.h"
#include "renderer/backend/vulkan/vulkan_command_buffer.h"
#include "renderer/backend/vulkan/vulkan_buffers.h"
#include "renderer/backend/vulkan/vulkan_render_pass.h"
#include "renderer/backend/vulkan/vulkan_graphics_pipeline.h"
#include "renderer/backend/vulkan/vulkan_descriptors.h"
#include "renderer/backend/vulkan/vulkan_framebuffer.h"
#include "renderer/backend/vulkan/vulkan_queue.h"
#include "renderer/backend/vulkan/vulkan_material.h"

namespace Phos {

VulkanRenderer::VulkanRenderer(const RendererConfig& config) {
    VulkanContext::init(config.window);

    m_graphics_queue = VulkanContext::device->get_graphics_queue();

    // Synchronization
    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    m_in_flight_fences.resize(Renderer::config().num_frames);
    for (std::size_t i = 0; i < Renderer::config().num_frames; ++i) {
        VK_CHECK(vkCreateFence(VulkanContext::device->handle(), &fence_create_info, nullptr, &m_in_flight_fences[i]));
    }

    // Frame descriptors
    m_allocator = std::make_shared<VulkanDescriptorAllocator>();

    m_camera_ubos.resize(Renderer::config().num_frames);
    m_lights_ubos.resize(Renderer::config().num_frames);
    m_frame_descriptor_sets.resize(Renderer::config().num_frames);

    for (uint32_t i = 0; i < Renderer::config().num_frames; ++i) {
        m_camera_ubos[i] = VulkanUniformBuffer::create<CameraUniformBuffer>();
        m_lights_ubos[i] = VulkanUniformBuffer::create<LightsUniformBuffer>();

        VkDescriptorBufferInfo camera_info{};
        camera_info.buffer = m_camera_ubos[i]->handle();
        camera_info.range = m_camera_ubos[i]->size();
        camera_info.offset = 0;

        VkDescriptorBufferInfo lights_info{};
        lights_info.buffer = m_lights_ubos[i]->handle();
        lights_info.range = m_lights_ubos[i]->size();
        lights_info.offset = 0;

        [[maybe_unused]] const bool built =
            VulkanDescriptorBuilder::begin(VulkanContext::descriptor_layout_cache, m_allocator)
                .bind_buffer(0, &camera_info, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                .bind_buffer(1, &lights_info, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .build(m_frame_descriptor_sets[i]);

        PHOS_ASSERT(built, "Error creating frame descriptor set");
    }

    // Create screen quad buffers
    const std::vector<ScreenQuadVertex> vertex_info = {
        {.position = glm::vec3(-1.0f, -1.0f, 0.0f), .texture_coord = glm::vec2(0.0f, 0.0f)},
        {.position = glm::vec3(1.0f, -1.0f, 0.0f), .texture_coord = glm::vec2(1.0f, 0.0f)},
        {.position = glm::vec3(-1.0f, 1.0f, 0.0f), .texture_coord = glm::vec2(0.0f, 1.0f)},
        {.position = glm::vec3(1.0f, 1.0f, 0.0f), .texture_coord = glm::vec2(1.0f, 1.0f)},
    };
    m_screen_quad_vertex = std::make_shared<VulkanVertexBuffer>(vertex_info);

    const std::vector<uint32_t> index_info = {0, 2, 1, 2, 3, 1};
    m_screen_quad_index = std::make_shared<VulkanIndexBuffer>(index_info);
}

VulkanRenderer::~VulkanRenderer() {
    vkDeviceWaitIdle(VulkanContext::device->handle());

    // Destroy synchronization elements
    for (uint32_t i = 0; i < Renderer::config().num_frames; ++i)
        vkDestroyFence(VulkanContext::device->handle(), m_in_flight_fences[i], nullptr);

    // Destroy ubos
    m_camera_ubos.clear();
    m_lights_ubos.clear();

    // Destroy screen quads
    m_screen_quad_vertex.reset();
    m_screen_quad_index.reset();

    // Destroy descriptor allocator
    m_allocator.reset();

    VulkanContext::free();
}

void VulkanRenderer::wait_idle() {
    vkDeviceWaitIdle(VulkanContext::device->handle());
}

void VulkanRenderer::begin_frame(const FrameInformation& info) {
    {
        PHOS_PROFILE_ZONE_SCOPED_NAMED("VulkanRenderer::begin_frame::waitForFences");
        vkWaitForFences(VulkanContext::device->handle(), 1, &m_in_flight_fences[m_current_frame], VK_TRUE, UINT64_MAX);
    }
    vkResetFences(VulkanContext::device->handle(), 1, &m_in_flight_fences[m_current_frame]);

    //
    // Update frame descriptors
    //

    // Camera
    auto projection = info.camera->projection_matrix();
    projection[1][1] *= -1;

    const auto camera_info = CameraUniformBuffer{
        .projection = projection,
        .view = info.camera->view_matrix(),
        // .view_projection = info.camera->projection_matrix() * info.camera->view_matrix(),
        .position = info.camera->position(),
    };
    m_camera_ubos[m_current_frame]->update(camera_info);

    // Lights
    LightsUniformBuffer lights_info{};

    int32_t directional_shadow_idx = 0;
    for (const auto& light : info.lights) {
        if (light->type() == Light::Type::Point && lights_info.number_point_lights < MAX_POINT_LIGHTS) {
            const auto& pl = std::dynamic_pointer_cast<PointLight>(light);
            lights_info.point_lights[lights_info.number_point_lights] = {
                .color = pl->color,
                .position = glm::vec4(pl->position, 1.0f),
                .intensity = pl->intensity,
            };
            lights_info.number_point_lights += 1;
        } else if (light->type() == Light::Type::Directional &&
                   lights_info.number_directional_lights < MAX_DIRECTIONAL_LIGHTS) {
            const auto& dl = std::dynamic_pointer_cast<DirectionalLight>(light);
            lights_info.directional_lights[lights_info.number_directional_lights] = {
                .color = dl->color,
                .direction = glm::vec4(dl->direction, 0.0f),
                .intensity = dl->intensity,
                .shadow_map_idx = dl->shadow_type != Light::ShadowType::None &&
                                          directional_shadow_idx + 1 <= static_cast<int32_t>(MAX_DIRECTIONAL_LIGHTS)
                                      ? directional_shadow_idx++
                                      : -1,
            };
            lights_info.number_directional_lights += 1;
        }
    }

    m_lights_ubos[m_current_frame]->update(lights_info);
}

void VulkanRenderer::end_frame() {
    m_current_frame = (m_current_frame + 1) % Renderer::config().num_frames;
}

void VulkanRenderer::submit_static_mesh(const std::shared_ptr<CommandBuffer>& command_buffer,
                                        const std::shared_ptr<Mesh>& mesh,
                                        const std::shared_ptr<Material>& material) {
    const auto& native_command_buffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(command_buffer);

    const auto& native_vertex = std::dynamic_pointer_cast<VulkanVertexBuffer>(mesh->vertex_buffer());
    const auto& native_index = std::dynamic_pointer_cast<VulkanIndexBuffer>(mesh->index_buffer());

    const auto& native_material = std::dynamic_pointer_cast<VulkanMaterial>(material);
    native_material->bind(native_command_buffer);

    VulkanRendererAPI::draw_indexed(native_command_buffer, native_vertex, native_index);
}

void VulkanRenderer::bind_graphics_pipeline(const std::shared_ptr<CommandBuffer>& command_buffer,
                                            const std::shared_ptr<GraphicsPipeline>& pipeline) {
    const auto& native_command_buffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(command_buffer);
    const auto& native_pipeline = std::dynamic_pointer_cast<VulkanGraphicsPipeline>(pipeline);

    native_pipeline->bind(native_command_buffer);

    // TODO: Should not be here
    {
        // Viewport and scissor info
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)native_pipeline->target_framebuffer()->width();
        viewport.height = (float)native_pipeline->target_framebuffer()->height();
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = {native_pipeline->target_framebuffer()->width(),
                          native_pipeline->target_framebuffer()->height()};

        vkCmdSetViewport(native_command_buffer->handle(), 0, 1, &viewport);
        vkCmdSetScissor(native_command_buffer->handle(), 0, 1, &scissor);
    }

    vkCmdBindDescriptorSets(native_command_buffer->handle(),
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            native_pipeline->layout(),
                            0, // Set = 0 for general frame descriptors
                            1,
                            &m_frame_descriptor_sets[m_current_frame],
                            0,
                            nullptr);
}

void VulkanRenderer::begin_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                                       const std::shared_ptr<RenderPass>& render_pass) {
    render_pass->begin(command_buffer);
}

void VulkanRenderer::end_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                                     const std::shared_ptr<RenderPass>& render_pass) {
    render_pass->end(command_buffer);
}

void VulkanRenderer::submit_command_buffer(const std::shared_ptr<CommandBuffer>& command_buffer) {
    const auto& native_command_buffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(command_buffer);

    const std::array<VkCommandBuffer, 1> command_buffers = {native_command_buffer->handle()};
    const std::vector<VkPipelineStageFlags> wait_stages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());
    info.pCommandBuffers = command_buffers.data();
    info.waitSemaphoreCount = 0;
    info.pWaitDstStageMask = wait_stages.data();
    info.signalSemaphoreCount = 0;

    m_graphics_queue->submit(info, m_in_flight_fences[m_current_frame]);
}

void VulkanRenderer::draw_screen_quad(const std::shared_ptr<CommandBuffer>& command_buffer) {
    const auto& native_command_buffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(command_buffer);
    VulkanRendererAPI::draw_indexed(native_command_buffer, m_screen_quad_vertex, m_screen_quad_index);
}

} // namespace Phos