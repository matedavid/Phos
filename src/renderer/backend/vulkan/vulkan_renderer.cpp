#include "vulkan_renderer.h"

#include "renderer/static_mesh.h"
#include "renderer/camera.h"

#include "renderer/backend/vulkan/vulkan_context.h"
#include "renderer/backend/vulkan/vulkan_renderer_api.h"
#include "renderer/backend/vulkan/vulkan_command_buffer.h"
#include "renderer/backend/vulkan/vulkan_buffers.h"
#include "renderer/backend/vulkan/vulkan_render_pass.h"
#include "renderer/backend/vulkan/vulkan_graphics_pipeline.h"
#include "renderer/backend/vulkan/vulkan_descriptors.h"
#include "renderer/backend/vulkan/vulkan_swapchain.h"
#include "renderer/backend/vulkan/vulkan_framebuffer.h"
#include "renderer/backend/vulkan/vulkan_queue.h"

namespace Phos {

VulkanRenderer::VulkanRenderer(const RendererConfig& config) {
    VulkanContext::init(config.window);

    // Swapchain
    m_swapchain = std::make_shared<VulkanSwapchain>();

    m_graphics_queue = VulkanContext::device->get_graphics_queue();
    m_presentation_queue = VulkanContext::device->get_presentation_queue();

    // Synchronization
    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VK_CHECK(
        vkCreateSemaphore(VulkanContext::device->handle(), &semaphore_create_info, nullptr, &image_available_semaphore))
    VK_CHECK(
        vkCreateSemaphore(VulkanContext::device->handle(), &semaphore_create_info, nullptr, &render_finished_semaphore))
    VK_CHECK(vkCreateFence(VulkanContext::device->handle(), &fence_create_info, nullptr, &in_flight_fence))

    // Frame descriptors
    m_allocator = std::make_shared<VulkanDescriptorAllocator>();

    m_camera_ubo = VulkanUniformBuffer::create<CameraUniformBuffer>();

    VkDescriptorBufferInfo camera_info{};
    camera_info.buffer = m_camera_ubo->handle();
    camera_info.range = m_camera_ubo->size();
    camera_info.offset = 0;

    // VkDescriptorBufferInfo lights_info{};

    bool built = VulkanDescriptorBuilder::begin(VulkanContext::descriptor_layout_cache, m_allocator)
                     .bind_buffer(0, camera_info, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                     .build(m_frame_descriptor_set);

    PS_ASSERT(built, "Error creating frame descriptor set")

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

    vkDestroySemaphore(VulkanContext::device->handle(), image_available_semaphore, nullptr);
    vkDestroySemaphore(VulkanContext::device->handle(), render_finished_semaphore, nullptr);
    vkDestroyFence(VulkanContext::device->handle(), in_flight_fence, nullptr);

    // Destroy camera ubo
    m_camera_ubo.reset();

    // Destroy screen quads
    m_screen_quad_vertex.reset();
    m_screen_quad_index.reset();

    // Destroy descriptor allocator
    m_allocator.reset();

    // Destroy swapchain
    m_swapchain.reset();

    VulkanContext::free();
}

void VulkanRenderer::begin_frame(const FrameInformation& info) {
    vkWaitForFences(VulkanContext::device->handle(), 1, &in_flight_fence, VK_TRUE, UINT64_MAX);

    auto projection = info.camera->projection_matrix();
    projection[1][1] *= -1;

    // Update frame descriptors
    const auto camera_info = CameraUniformBuffer{
        .projection = projection,
        .view = info.camera->view_matrix(),
        // .view_projection = info.camera->projection_matrix() * info.camera->view_matrix(),
        .position = info.camera->position(),
    };
    m_camera_ubo->update(camera_info);

    // Acquire next swapchain image
    m_swapchain->acquire_next_image(image_available_semaphore, VK_NULL_HANDLE);
    vkResetFences(VulkanContext::device->handle(), 1, &in_flight_fence);
}

void VulkanRenderer::end_frame() {
    const std::vector<VkSemaphore> signal_semaphores = {render_finished_semaphore};

    // Present image
    const auto result =
        m_presentation_queue->submitKHR(m_swapchain, m_swapchain->get_current_image_idx(), signal_semaphores);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        m_swapchain->recreate();
    } else if (result != VK_SUCCESS) {
        PS_FAIL("Failed to present image")
    }
}

void VulkanRenderer::submit_static_mesh(const std::shared_ptr<CommandBuffer>& command_buffer,
                                        const std::shared_ptr<StaticMesh>& mesh) {
    const auto& native_command_buffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(command_buffer);

    for (const auto& sub : mesh->get_sub_meshes()) {
        const auto& native_vertex_buffer = std::dynamic_pointer_cast<VulkanVertexBuffer>(sub->get_vertex_buffer());
        const auto& native_index_buffer = std::dynamic_pointer_cast<VulkanIndexBuffer>(sub->get_index_buffer());

        VulkanRendererAPI::draw_indexed(native_command_buffer, native_vertex_buffer, native_index_buffer);
    }
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
        viewport.width = (float)m_swapchain->get_current_framebuffer()->width();
        viewport.height = (float)m_swapchain->get_current_framebuffer()->height();
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = {m_swapchain->get_current_framebuffer()->width(),
                          m_swapchain->get_current_framebuffer()->height()};

        vkCmdSetViewport(native_command_buffer->handle(), 0, 1, &viewport);
        vkCmdSetScissor(native_command_buffer->handle(), 0, 1, &scissor);
    }

    vkCmdBindDescriptorSets(native_command_buffer->handle(),
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            native_pipeline->layout(),
                            0, // Set = 0 for general frame descriptors
                            1,
                            &m_frame_descriptor_set,
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

void VulkanRenderer::record_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                                        const std::shared_ptr<RenderPass>& render_pass,
                                        const std::function<void(void)>& func) {
    // Begin Render Pass
    begin_render_pass(command_buffer, render_pass);

    // Call Record Function
    func();

    // End Render Pass
    end_render_pass(command_buffer, render_pass);
}

void VulkanRenderer::submit_command_buffer(const std::shared_ptr<CommandBuffer>& command_buffer) {
    const auto& native_command_buffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(command_buffer);

    const std::vector<VkPipelineStageFlags> wait_stages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    const std::vector<VkSemaphore> wait_semaphores = {image_available_semaphore};
    const std::vector<VkSemaphore> signal_semaphores = {render_finished_semaphore};

    const std::array<VkCommandBuffer, 1> command_buffers = {native_command_buffer->handle()};

    VkSubmitInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.waitSemaphoreCount = (uint32_t)wait_semaphores.size();
    info.pWaitSemaphores = wait_semaphores.data();
    info.pWaitDstStageMask = wait_stages.data();
    info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());
    info.pCommandBuffers = command_buffers.data();
    info.signalSemaphoreCount = (uint32_t)signal_semaphores.size();
    info.pSignalSemaphores = signal_semaphores.data();

    m_graphics_queue->submit(info, in_flight_fence);
}

void VulkanRenderer::draw_screen_quad(const std::shared_ptr<CommandBuffer>& command_buffer) {
    const auto& native_command_buffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(command_buffer);
    VulkanRendererAPI::draw_indexed(native_command_buffer, m_screen_quad_vertex, m_screen_quad_index);
}

std::shared_ptr<Framebuffer> VulkanRenderer::current_frame_framebuffer() {
    return m_swapchain->get_current_framebuffer();
}

std::shared_ptr<Framebuffer> VulkanRenderer::presentation_framebuffer() {
    return m_swapchain->get_target_framebuffer();
}

} // namespace Phos