#include "vulkan_context.h"

#include <ranges>
#include <glm/gtc/matrix_transform.hpp>

#include "core/window.h"

#include "renderer/vulkan_shader_module.h"

VulkanContext::VulkanContext(const std::shared_ptr<Window>& window) {
    m_instance = std::make_unique<VulkanInstance>(window);

    const std::vector<std::string_view> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    //    const auto physical_devices = m_instance->get_physical_devices();
    //    const auto selected_physical_device = select_physical_device(physical_devices, device_extensions);

    const auto device_requirements = VulkanPhysicalDevice::Requirements{
        .graphics = true,
        .transfer = true,
        .presentation = true,
        .surface = m_instance->get_surface(),

        .extensions = device_extensions,
    };
    m_device = std::make_shared<VulkanDevice>(m_instance, device_requirements);

    m_graphics_queue = m_device->get_graphics_queue();
    m_presentation_queue = m_device->get_presentation_queue();

    m_swapchain = std::make_shared<VulkanSwapchain>(m_device, m_instance->get_surface(), window);

    m_layout_cache = std::make_shared<VulkanDescriptorLayoutCache>(m_device);
    m_allocator = std::make_shared<VulkanDescriptorAllocator>(m_device);

    const auto vertex = std::make_shared<VulkanShaderModule>(
        "../assets/shaders/vertex.spv", VulkanShaderModule::Stage::Vertex, m_device, m_layout_cache);
    const auto fragment = std::make_shared<VulkanShaderModule>(
        "../assets/shaders/fragment.spv", VulkanShaderModule::Stage::Fragment, m_device, m_layout_cache);

    m_render_pass = std::make_shared<VulkanRenderPass>(m_device);

    const auto pipeline_description = VulkanGraphicsPipeline::Description{
        .shader_modules = {vertex, fragment},
        .render_pass = m_render_pass,
    };
    m_pipeline = std::make_shared<VulkanGraphicsPipeline>(m_device, pipeline_description);

    m_command_buffer = m_device->create_command_buffer(VulkanQueue::Type::Graphics);

    for (const auto& view : m_swapchain->get_image_views()) {
        const std::vector<VkImageView> attachments = {view};
        const auto framebuffer = std::make_shared<VulkanFramebuffer>(
            m_device, m_render_pass, window->get_width(), window->get_height(), attachments);

        m_present_framebuffers.push_back(framebuffer);
    }

    // Synchronization
    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VK_CHECK(vkCreateSemaphore(m_device->handle(), &semaphoreCreateInfo, nullptr, &image_available_semaphore))
    VK_CHECK(vkCreateSemaphore(m_device->handle(), &semaphoreCreateInfo, nullptr, &render_finished_semaphore))
    VK_CHECK(vkCreateFence(m_device->handle(), &fenceCreateInfo, nullptr, &in_flight_fence))

    // Vertex and Index buffers
    const std::vector<Vertex> data = {
        {.position = {-0.5f, -0.5f, 0.0f}},
        {.position = {0.5f, -0.5f, 0.0f}},
        {.position = {-0.5f, 0.5f, 0.0f}},
        {.position = {0.5f, 0.5f, 0.0f}},
    };

    m_vertex_buffer = std::make_unique<VulkanVertexBuffer<Vertex>>(m_device, data);

    const std::vector<uint32_t> indices = {0, 2, 1, 1, 2, 3};
    m_index_buffer = std::make_unique<VulkanIndexBuffer>(m_device, indices);

    // Uniform buffer stuff
    // =======================

    m_color_ubo = std::make_shared<VulkanUniformBuffer<ColorUniformBuffer>>(m_device);

    VkDescriptorBufferInfo color_info{};
    color_info.buffer = m_color_ubo->handle();
    color_info.offset = 0;
    color_info.range = m_color_ubo->size();

    bool result = VulkanDescriptorBuilder::begin(m_device, m_layout_cache, m_allocator)
                      .bind_buffer(0, color_info, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
                      .build(m_uniform_buffer_set);

    CORE_ASSERT(result, "Error creating descriptor set")
    // =======================
}

VulkanContext::~VulkanContext() {
    vkDeviceWaitIdle(m_device->handle());

    vkDestroySemaphore(m_device->handle(), image_available_semaphore, nullptr);
    vkDestroySemaphore(m_device->handle(), render_finished_semaphore, nullptr);
    vkDestroyFence(m_device->handle(), in_flight_fence, nullptr);
}

void VulkanContext::update() {
    vkWaitForFences(m_device->handle(), 1, &in_flight_fence, VK_TRUE, UINT64_MAX);

    const auto next_image = m_swapchain->acquire_next_image(image_available_semaphore);

    vkResetFences(m_device->handle(), 1, &in_flight_fence);

    // Update uniform buffer
    m_color_ubo->update({
        .color = glm::vec4(0.7f, 0.3f, 0.4f, 1.0f),
    });

    // Record command buffer
    const auto& command_buffer = m_command_buffer;

    command_buffer->begin();
    m_render_pass->begin(command_buffer, m_present_framebuffers[next_image]);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_present_framebuffers[0]->width();
    viewport.height = (float)m_present_framebuffers[0]->height();
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = {m_present_framebuffers[next_image]->width(), m_present_framebuffers[next_image]->height()};

    m_pipeline->bind(command_buffer);
    vkCmdSetViewport(command_buffer->handle(), 0, 1, &viewport);
    vkCmdSetScissor(command_buffer->handle(), 0, 1, &scissor);

    m_vertex_buffer->bind(command_buffer);
    m_index_buffer->bind(command_buffer);

    // Uniform buffer
    vkCmdBindDescriptorSets(command_buffer->handle(),
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_pipeline->layout(),
                            0,
                            1,
                            &m_uniform_buffer_set,
                            0,
                            nullptr);
    // =================

    vkCmdDrawIndexed(m_command_buffer->handle(), m_index_buffer->get_count(), 1, 0, 0, 0);

    m_render_pass->end(command_buffer);
    command_buffer->end();
    // ===========================

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
    const auto result = m_presentation_queue->submitKHR(m_swapchain, next_image, signal_semaphores);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        // TODO: recreate swapchain
    } else if (result != VK_SUCCESS) {
        CORE_FAIL("Failed to present image")
    }
}
