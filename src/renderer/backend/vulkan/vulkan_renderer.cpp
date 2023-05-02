#include "vulkan_renderer.h"

#include "renderer/backend/vulkan/vulkan_context.h"

#include "renderer/model.h"

namespace Phos {

VulkanRenderer::VulkanRenderer() {
    // From previous VulkanContext
    m_graphics_queue = VulkanContext::device->get_graphics_queue();
    m_presentation_queue = VulkanContext::device->get_presentation_queue();

    m_allocator = std::make_shared<VulkanDescriptorAllocator>();

    const auto vertex =
        std::make_shared<VulkanShaderModule>("../assets/shaders/vertex.spv", VulkanShaderModule::Stage::Vertex);
    const auto fragment =
        std::make_shared<VulkanShaderModule>("../assets/shaders/fragment.spv", VulkanShaderModule::Stage::Fragment);

    m_render_pass = std::make_shared<VulkanRenderPass>();

    const auto pipeline_description = VulkanGraphicsPipeline::Description{
        .shader_modules = {vertex, fragment},
        .render_pass = m_render_pass,
    };
    m_pipeline = std::make_shared<VulkanGraphicsPipeline>(pipeline_description);

    m_command_buffer = VulkanContext::device->create_command_buffer(VulkanQueue::Type::Graphics);

    // Swapchain
    m_swapchain = std::make_shared<VulkanSwapchain>();
    m_swapchain->specify_render_pass(m_render_pass);

    // Synchronization
    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VK_CHECK(
        vkCreateSemaphore(VulkanContext::device->handle(), &semaphoreCreateInfo, nullptr, &image_available_semaphore))
    VK_CHECK(
        vkCreateSemaphore(VulkanContext::device->handle(), &semaphoreCreateInfo, nullptr, &render_finished_semaphore))
    VK_CHECK(vkCreateFence(VulkanContext::device->handle(), &fenceCreateInfo, nullptr, &in_flight_fence))

    // Vertex and Index buffers
    const std::vector<Vertex> data = {
        {.position = {-0.5f, -0.5f, 0.6f}, .texture_coords = {0.0f, 0.0f}},
        {.position = {0.5f, -0.5f, 0.6f}, .texture_coords = {1.0f, 0.0f}},
        {.position = {-0.5f, 0.5f, 0.6f}, .texture_coords = {0.0f, 1.0f}},
        {.position = {0.5f, 0.5f, 0.6f}, .texture_coords = {1.0f, 1.0f}},
    };

    m_vertex_buffer = std::make_unique<VulkanVertexBuffer<Vertex>>(data);

    const std::vector<Vertex> data2 = {
        {.position = {-0.5f + 0.25f, -0.5f + 0.25f, 0.5f}, .texture_coords = {0.0f, 0.0f}},
        {.position = {0.5f + 0.25f, -0.5f + 0.25f, 0.5f}, .texture_coords = {1.0f, 0.0f}},
        {.position = {-0.5f + 0.25f, 0.5f + 0.25f, 0.5f}, .texture_coords = {0.0f, 1.0f}},
        {.position = {0.5f + 0.25f, 0.5f + 0.25f, 0.5f}, .texture_coords = {1.0f, 1.0f}},
    };

    m_vertex_buffer_2 = std::make_unique<VulkanVertexBuffer<Vertex>>(data2);

    const std::vector<uint32_t> indices = {0, 2, 1, 1, 2, 3};
    m_index_buffer = std::make_unique<VulkanIndexBuffer>(indices);

    // Texture
    m_texture = std::make_unique<VulkanTexture>("../assets/texture.jpg");

    // Uniform buffer stuff
    // =======================

    m_color_ubo = std::make_shared<VulkanUniformBuffer<ColorUniformBuffer>>();

    VkDescriptorBufferInfo color_info{};
    color_info.buffer = m_color_ubo->handle();
    color_info.offset = 0;
    color_info.range = m_color_ubo->size();

    VkDescriptorImageInfo image_info{};
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info.imageView = m_texture->image_view();
    image_info.sampler = m_texture->sampler();

    bool result =
        VulkanDescriptorBuilder::begin(VulkanContext::descriptor_layout_cache, m_allocator)
            .bind_buffer(0, color_info, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .bind_image(1, image_info, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build(m_uniform_buffer_set);

    PS_ASSERT(result, "Error creating descriptor set")
    // =======================

    // Model test
    [[maybe_unused]] auto model = Model("../assets/suzanne.fbx", false);
}

VulkanRenderer::~VulkanRenderer() {
    vkDeviceWaitIdle(VulkanContext::device->handle());

    vkDestroySemaphore(VulkanContext::device->handle(), image_available_semaphore, nullptr);
    vkDestroySemaphore(VulkanContext::device->handle(), render_finished_semaphore, nullptr);
    vkDestroyFence(VulkanContext::device->handle(), in_flight_fence, nullptr);
}

void VulkanRenderer::update() {
    vkWaitForFences(VulkanContext::device->handle(), 1, &in_flight_fence, VK_TRUE, UINT64_MAX);

    m_swapchain->acquire_next_image(image_available_semaphore, VK_NULL_HANDLE);
    const auto& swapchain_framebuffer = m_swapchain->get_current_framebuffer();

    vkResetFences(VulkanContext::device->handle(), 1, &in_flight_fence);

    // Update uniform buffer
    m_color_ubo->update({
        .color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
    });

    // Record command buffer
    const auto& command_buffer = m_command_buffer;

    command_buffer->record([&]() {
        m_render_pass->begin(*command_buffer, *swapchain_framebuffer);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapchain_framebuffer->width();
        viewport.height = (float)swapchain_framebuffer->height();
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = {swapchain_framebuffer->width(), swapchain_framebuffer->height()};

        m_pipeline->bind(command_buffer);
        vkCmdSetViewport(command_buffer->handle(), 0, 1, &viewport);
        vkCmdSetScissor(command_buffer->handle(), 0, 1, &scissor);

        // Descriptor sets
        vkCmdBindDescriptorSets(command_buffer->handle(),
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_pipeline->layout(),
                                0,
                                1,
                                &m_uniform_buffer_set,
                                0,
                                nullptr);

        m_vertex_buffer_2->bind(command_buffer);
        m_index_buffer->bind(command_buffer);
        vkCmdDrawIndexed(m_command_buffer->handle(), m_index_buffer->get_count(), 1, 0, 0, 0);

        m_vertex_buffer->bind(command_buffer);
        m_index_buffer->bind(command_buffer);
        vkCmdDrawIndexed(m_command_buffer->handle(), m_index_buffer->get_count(), 1, 0, 0, 0);

        m_render_pass->end(*command_buffer);
    });

    // Submit command buffer
    const std::vector<VkPipelineStageFlags> wait_stages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    const std::vector<VkSemaphore> wait_semaphores = {image_available_semaphore};
    const std::vector<VkSemaphore> signal_semaphores = {render_finished_semaphore};

    // Submit queue
    const std::array<VkCommandBuffer, 1> command_buffers = {command_buffer->handle()};

    VkSubmitInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.waitSemaphoreCount = (uint32_t)wait_semaphores.size();
    info.pWaitSemaphores = wait_semaphores.data();
    info.pWaitDstStageMask = wait_stages.data();
    info.commandBufferCount = 1;
    info.pCommandBuffers = command_buffers.data();
    info.signalSemaphoreCount = (uint32_t)signal_semaphores.size();
    info.pSignalSemaphores = signal_semaphores.data();

    m_graphics_queue->submit(info, in_flight_fence);

    // Present image
    const auto result =
        m_presentation_queue->submitKHR(m_swapchain, m_swapchain->get_current_image_idx(), signal_semaphores);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        m_swapchain->recreate();
    } else if (result != VK_SUCCESS) {
        PS_FAIL("Failed to present image")
    }
}

} // namespace Phos
