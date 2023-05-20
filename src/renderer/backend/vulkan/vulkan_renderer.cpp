/*
#include "vulkan_renderer.h"

#include "renderer/backend/vulkan/vulkan_context.h"

#include "renderer/model.h"
#include "core/window.h"

namespace Phos {

VulkanRenderer::VulkanRenderer() {
    VulkanContext::window->add_event_callback_func([&](Event& event) { on_event(event); });

    // From previous VulkanContext
    m_graphics_queue = VulkanContext::device->get_graphics_queue();
    m_presentation_queue = VulkanContext::device->get_presentation_queue();

    m_allocator = std::make_shared<VulkanDescriptorAllocator>();

    // Swapchain
    m_swapchain = std::make_shared<VulkanSwapchain>();

    const auto vertex =
        std::make_shared<VulkanShaderModule>("../assets/shaders/vertex.spv", VulkanShaderModule::Stage::Vertex);
    const auto fragment =
        std::make_shared<VulkanShaderModule>("../assets/shaders/fragment.spv", VulkanShaderModule::Stage::Fragment);

    const auto pipeline_description = VulkanGraphicsPipeline::Description{
        .vertex_shader = vertex,
        .fragment_shader = fragment,
        .target_framebuffer = m_swapchain->get_target_framebuffer(),
    };
    m_pipeline = std::make_shared<VulkanGraphicsPipeline>(pipeline_description);

    const auto flat_color_vertex = std::make_shared<VulkanShaderModule>("../assets/shaders/flat_color_vertex.spv",
                                                                        VulkanShaderModule::Stage::Vertex);

    const auto flat_color_fragment = std::make_shared<VulkanShaderModule>("../assets/shaders/flat_color_fragment.spv",
                                                                          VulkanShaderModule::Stage::Fragment);

    const auto flat_color_pipeline_description = VulkanGraphicsPipeline::Description{
        .vertex_shader = flat_color_vertex,
        .fragment_shader = flat_color_fragment,
        .target_framebuffer = m_swapchain->get_target_framebuffer(),
    };
    m_flat_color_pipeline = std::make_shared<VulkanGraphicsPipeline>(flat_color_pipeline_description);

    m_render_pass = std::make_shared<VulkanRenderPass>(VulkanRenderPass::Description{
        .debug_name = "Presentation",
        .presentation_target = true,
        .target_framebuffer = m_swapchain->get_target_framebuffer(),
    });

    m_command_buffer = VulkanContext::device->create_command_buffer(VulkanQueue::Type::Graphics);

    // Geometry pass
    const auto position_texture =
        std::make_shared<VulkanTexture>(VulkanContext::window->get_width(), VulkanContext::window->get_height());
    const auto diffuse_texture =
        std::make_shared<VulkanTexture>(VulkanContext::window->get_width(), VulkanContext::window->get_height());

    const auto geometry_framebuffer_description = VulkanFramebuffer::Description{
        .attachments =
            {
                VulkanFramebuffer::Attachment{
                    .image = position_texture->get_image(),
                    .load_operation = LoadOperation::Clear,
                    .store_operation = StoreOperation::Store,
                    .clear_value = glm::vec3(0.0f),
                },
                VulkanFramebuffer::Attachment{
                    .image = diffuse_texture->get_image(),
                    .load_operation = LoadOperation::Clear,
                    .store_operation = StoreOperation::Store,
                    .clear_value = glm::vec3(0.0f),
                },
            },
    };
    const auto geometry_framebuffer = std::make_shared<VulkanFramebuffer>(geometry_framebuffer_description);

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

    // Uniform buffer stuff
    // =======================

    m_camera_ubo = std::make_shared<VulkanUniformBuffer<CameraUniformBuffer>>();
    m_color_ubo = std::make_shared<VulkanUniformBuffer<ColorUniformBuffer>>();
    m_lights_ubo = std::make_shared<VulkanUniformBuffer<LightsUniformBuffer>>();

    VkDescriptorBufferInfo camera_info{};
    camera_info.buffer = m_camera_ubo->handle();
    camera_info.offset = 0;
    camera_info.range = m_camera_ubo->size();

    VkDescriptorBufferInfo color_info{};
    color_info.buffer = m_color_ubo->handle();
    color_info.offset = 0;
    color_info.range = m_color_ubo->size();

    VkDescriptorBufferInfo lights_info{};
    lights_info.buffer = m_lights_ubo->handle();
    lights_info.offset = 0;
    lights_info.range = m_lights_ubo->size();

    const bool result1 = VulkanDescriptorBuilder::begin(VulkanContext::descriptor_layout_cache, m_allocator)
                             .bind_buffer(0, camera_info, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                             .build(m_vertex_shader_set);

    const bool result2 =
        VulkanDescriptorBuilder::begin(VulkanContext::descriptor_layout_cache, m_allocator)
            .bind_buffer(0, color_info, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .bind_buffer(1, lights_info, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build(m_fragment_shader_set);

    PS_ASSERT(result1 && result2, "Error creating descriptor set")
    // =======================

    // Model
    m_model = std::make_shared<Model>("../assets/suzanne.fbx", false);
    m_cube = std::make_shared<Model>("../assets/cube.fbx", false);
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

    // Update uniform buffers
    const float aspect_ratio =
        static_cast<float>(swapchain_framebuffer->width()) / static_cast<float>(swapchain_framebuffer->height());

    auto projection = glm::perspective(glm::radians(90.0f), aspect_ratio, 0.001f, 40.0f);
    projection[1][1] *= -1;

    auto view = glm::mat4(1.0f);
    view = glm::translate(view, -m_camera_info.position);
    view = glm::rotate(view, m_camera_info.rotation.y, glm::vec3(1.0f, 0.0f, 0.0f));
    view = glm::rotate(view, -m_camera_info.rotation.x, glm::vec3(0.0f, 1.0f, 0.0f));

    m_camera_ubo->update({
        .projection = projection,
        .view = view,
        .position = glm::vec3(glm::inverse(view)[3]),
    });

    m_color_ubo->update({
        .color = glm::vec4(1.0f),
    });

    update_light_info();

    // Record command buffer
    const auto& command_buffer = m_command_buffer;

    command_buffer->record([&]() {
        // Draw model
        // ==========================
        m_render_pass->begin(*command_buffer, swapchain_framebuffer);

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
        std::vector<VkDescriptorSet> descriptor_sets = {m_vertex_shader_set, m_fragment_shader_set};

        vkCmdBindDescriptorSets(command_buffer->handle(),
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_pipeline->layout(),
                                0,
                                static_cast<uint32_t>(descriptor_sets.size()),
                                descriptor_sets.data(),
                                0,
                                nullptr);

        for (const auto& mesh : m_model->get_meshes()) {
            const auto& vertex_buffer = mesh->get_vertex_buffer();
            const auto& index_buffer = mesh->get_index_buffer();

            vertex_buffer->bind(command_buffer);
            index_buffer->bind(command_buffer);

            vkCmdDrawIndexed(m_command_buffer->handle(), index_buffer->get_count(), 1, 0, 0, 0);
        }

        // Draw lights
        // ==========================
        m_flat_color_pipeline->bind(command_buffer);

        vkCmdBindDescriptorSets(command_buffer->handle(),
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_flat_color_pipeline->layout(),
                                0,
                                1,
                                &m_vertex_shader_set,
                                0,
                                nullptr);

        for (int i = 0; i < light_info.count; ++i) {
            const glm::vec3 position = glm::vec3(light_info.positions[i]);
            const glm::vec4 color = light_info.colors[i];

            glm::mat4 model{1.0f};
            model = glm::translate(model, position);
            model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

            ModelInfoPushConstant constants = {
                .model = model,
                .color = color,
            };

            vkCmdPushConstants(command_buffer->handle(),
                               m_flat_color_pipeline->layout(),
                               VK_SHADER_STAGE_VERTEX_BIT,
                               0,
                               sizeof(ModelInfoPushConstant),
                               &constants);

            const auto& mesh = m_cube->get_meshes()[0];

            const auto& vertex_buffer = mesh->get_vertex_buffer();
            const auto& index_buffer = mesh->get_index_buffer();

            vertex_buffer->bind(command_buffer);
            index_buffer->bind(command_buffer);

            vkCmdDrawIndexed(m_command_buffer->handle(), index_buffer->get_count(), 1, 0, 0, 0);
        }
        // ==========================

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

void VulkanRenderer::on_event(Event& event) {
    if (event.get_type() == EventType::MouseMoved) {
        auto mouse_moved = dynamic_cast<MouseMovedEvent&>(event);

        double x = mouse_moved.get_xpos();
        double y = mouse_moved.get_ypos();

        if (Input::is_mouse_button_pressed(MouseButton::Left)) {
            float x_rotation = 0.0f;
            float y_rotation = 0.0f;

            if (x > m_camera_info.mouse_pos.x) {
                x_rotation -= 0.03f;
            } else if (x < m_camera_info.mouse_pos.x) {
                x_rotation += 0.03f;
            }

            if (y > m_camera_info.mouse_pos.y) {
                y_rotation += 0.03f;
            } else if (y < m_camera_info.mouse_pos.y) {
                y_rotation -= 0.03f;
            }

            m_camera_info.rotation += glm::vec2{x_rotation, y_rotation};
        }

        m_camera_info.mouse_pos = glm::vec2(x, y);

    } else if (event.get_type() == EventType::KeyPressed) {
        auto key_pressed = dynamic_cast<KeyPressedEvent&>(event);

        if (key_pressed.get_key() == Key::W) {
            m_camera_info.position.z += -1;
        } else if (key_pressed.get_key() == Key::S) {
            m_camera_info.position.z += 1;
        } else if (key_pressed.get_key() == Key::A) {
            m_camera_info.position.x += -1;
        } else if (key_pressed.get_key() == Key::D) {
            m_camera_info.position.x += 1;
        }
    }
}

void VulkanRenderer::update_light_info() {
    light_info.count = 5;

    light_info.positions[0] = glm::vec4(-2.0f, 0.0f, 0.0f, 0.0f);
    light_info.colors[0] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

    light_info.positions[1] = glm::vec4(0.0f, 1.0f, -1.0f, 0.0f);
    light_info.colors[1] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

    light_info.positions[2] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    light_info.colors[2] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

    light_info.positions[3] = glm::vec4(-1.0f, -1.0f, -1.0f, 0.0f);
    light_info.colors[3] = glm::vec4(0.2f, 0.5f, 0.3f, 1.0f);

    light_info.positions[4] = glm::vec4(-1.0f, -2.0f, 1.0f, 0.0);
    light_info.colors[4] = glm::vec4(0.1f, 0.2f, 0.3f, 1.0f);

    m_lights_ubo->update(light_info);
}

} // namespace Phos

 */