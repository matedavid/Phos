#include "deferred_renderer.h"

#include "renderer/backend/vulkan/vulkan_context.h"
#include "renderer/backend/vulkan/vulkan_renderer_api.h"

#include "renderer/model.h"
#include "core/window.h"

namespace Phos {

DeferredRenderer::DeferredRenderer() {
    VulkanContext::window->add_event_callback_func([&](Event& event) { on_event(event); });

    m_graphics_queue = VulkanContext::device->get_graphics_queue();
    m_presentation_queue = VulkanContext::device->get_presentation_queue();

    // Swapchain
    m_swapchain = std::make_shared<VulkanSwapchain>();

    // Command buffer
    m_command_buffer = std::make_shared<VulkanCommandBuffer>();

    m_allocator = std::make_shared<VulkanDescriptorAllocator>();

    const auto width = VulkanContext::window->get_width();
    const auto height = VulkanContext::window->get_height();

    // Geometry pass
    {
        m_position_texture = std::make_shared<VulkanTexture>(width, height);
        m_normal_texture = std::make_shared<VulkanTexture>(width, height);
        m_color_specular_texture = std::make_shared<VulkanTexture>(width, height);

        const VulkanImage::Description depth_image_description = {
            .width = width,
            .height = height,
            .type = VulkanImage::Type::Image2D,
            .format = VulkanImage::Format::D32_SFLOAT,
            .transfer = false,
            .attachment = true,
        };
        const auto depth_image = std::make_shared<VulkanImage>(depth_image_description);

        const auto position_attachment = VulkanFramebuffer::Attachment{
            .image = m_position_texture->get_image(),
            .load_operation = LoadOperation::Clear,
            .store_operation = StoreOperation::Store,
            .clear_value = glm::vec3(0.0f),
        };
        const auto normal_attachment = VulkanFramebuffer::Attachment{
            .image = m_normal_texture->get_image(),
            .load_operation = LoadOperation::Clear,
            .store_operation = StoreOperation::Store,
            .clear_value = glm::vec3(0.0f),
        };
        const auto color_specular_attachment = VulkanFramebuffer::Attachment{
            .image = m_color_specular_texture->get_image(),
            .load_operation = LoadOperation::Clear,
            .store_operation = StoreOperation::Store,
            .clear_value = glm::vec3(0.0f),
        };
        const auto depth_attachment = VulkanFramebuffer::Attachment{
            .image = depth_image,
            .load_operation = LoadOperation::Clear,
            .store_operation = StoreOperation::DontCare,
            .clear_value = glm::vec3(1.0f),
        };

        m_geometry_framebuffer = std::make_shared<VulkanFramebuffer>(VulkanFramebuffer::Description{
            .attachments = {position_attachment, normal_attachment, color_specular_attachment, depth_attachment},
        });

        m_geometry_pipeline = std::make_shared<VulkanGraphicsPipeline>(VulkanGraphicsPipeline::Description{
            .shader = std::make_shared<VulkanShader>("../assets/shaders/geometry_vertex.spv",
                                                     "../assets/shaders/geometry_fragment.spv"),
            .target_framebuffer = m_geometry_framebuffer,
        });

        m_geometry_pass = std::make_shared<VulkanRenderPass>(VulkanRenderPass::Description{
            .debug_name = "Deferred-Geometry",
            .target_framebuffer = m_geometry_framebuffer,
        });
    }

    // Lighting pass
    {
        m_lighting_pipeline = std::make_shared<VulkanGraphicsPipeline>(VulkanGraphicsPipeline::Description{
            .shader = std::make_shared<VulkanShader>("../assets/shaders/lighting_vertex.spv",
                                                     "../assets/shaders/lighting_fragment.spv"),
            .target_framebuffer = m_swapchain->get_target_framebuffer(),
        });

        m_lighting_pass = std::make_shared<VulkanRenderPass>(VulkanRenderPass::Description{
            .debug_name = "Deferred-Lighting",
            .presentation_target = true,
        });
    }

    // Flat color pass
    {
        m_flat_color_pipeline = std::make_shared<VulkanGraphicsPipeline>(VulkanGraphicsPipeline::Description{
            .shader = std::make_shared<VulkanShader>("../assets/shaders/flat_color_vertex.spv",
                                                     "../assets/shaders/flat_color_fragment.spv"),
            .target_framebuffer = m_geometry_framebuffer,
        });
    }

    // Create Quad buffers
    {
        const std::vector<DeferredVertex> vertex_info = {
            {.position = glm::vec3(-1.0f, -1.0f, 0.0f), .texture_coord = glm::vec2(0.0f, 0.0f)},
            {.position = glm::vec3(1.0f, -1.0f, 0.0f), .texture_coord = glm::vec2(1.0f, 0.0f)},
            {.position = glm::vec3(-1.0f, 1.0f, 0.0f), .texture_coord = glm::vec2(0.0f, 1.0f)},
            {.position = glm::vec3(1.0f, 1.0f, 0.0f), .texture_coord = glm::vec2(1.0f, 1.0f)},
        };
        m_quad_vertex = std::make_shared<VulkanVertexBuffer>(vertex_info);

        const std::vector<uint32_t> index_info = {0, 2, 1, 2, 3, 1};
        m_quad_index = std::make_shared<VulkanIndexBuffer>(index_info);
    }

    // Uniform buffers
    m_camera_ubo = std::make_shared<VulkanUniformBuffer<CameraUniformBuffer>>();
    m_lights_ubo = std::make_shared<VulkanUniformBuffer<LightsUniformBuffer>>();

    VkDescriptorBufferInfo camera_info{};
    camera_info.buffer = m_camera_ubo->handle();
    camera_info.range = m_camera_ubo->size();
    camera_info.offset = 0;

    VkDescriptorBufferInfo lights_info{};
    lights_info.buffer = m_lights_ubo->handle();
    lights_info.range = m_lights_ubo->size();
    lights_info.offset = 0;

    VkDescriptorImageInfo position_map_info{};
    position_map_info.imageView = std::dynamic_pointer_cast<VulkanImage>(m_position_texture->get_image())->view();
    position_map_info.sampler = m_position_texture->sampler();
    position_map_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDescriptorImageInfo normal_map_info{};
    normal_map_info.imageView = std::dynamic_pointer_cast<VulkanImage>(m_normal_texture->get_image())->view();
    normal_map_info.sampler = m_normal_texture->sampler();
    normal_map_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDescriptorImageInfo color_specular_map_info{};
    color_specular_map_info.imageView =
        std::dynamic_pointer_cast<VulkanImage>(m_color_specular_texture->get_image())->view();
    color_specular_map_info.sampler = m_color_specular_texture->sampler();
    color_specular_map_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    bool result1 = VulkanDescriptorBuilder::begin(VulkanContext::descriptor_layout_cache, m_allocator)
                       .bind_buffer(0, camera_info, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                       .build(m_camera_set);

    bool result2 =
        VulkanDescriptorBuilder::begin(VulkanContext::descriptor_layout_cache, m_allocator)
            .bind_buffer(0, lights_info, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .bind_image(1, position_map_info, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .bind_image(2, normal_map_info, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .bind_image(
                3, color_specular_map_info, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build(m_lighting_fragment_set);

    PS_ASSERT(result1 && result2, "Error creating descriptor set")

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

    // Model
    m_model = std::make_shared<Model>("../assets/suzanne.fbx", false);
    m_cube = std::make_shared<Model>("../assets/cube.fbx", false);
}

DeferredRenderer::~DeferredRenderer() {
    vkDeviceWaitIdle(VulkanContext::device->handle());

    vkDestroySemaphore(VulkanContext::device->handle(), image_available_semaphore, nullptr);
    vkDestroySemaphore(VulkanContext::device->handle(), render_finished_semaphore, nullptr);
    vkDestroyFence(VulkanContext::device->handle(), in_flight_fence, nullptr);
}

void DeferredRenderer::update() {
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

    update_light_info();

    // Viewport and scissor info
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

    // Record command buffer
    const auto& command_buffer = m_command_buffer;

    command_buffer->record([&]() {
        // Geometry pass
        // =============
        {
            m_geometry_pass->begin(command_buffer);

            // Draw models
            m_geometry_pipeline->bind(command_buffer);
            vkCmdSetViewport(command_buffer->handle(), 0, 1, &viewport);
            vkCmdSetScissor(command_buffer->handle(), 0, 1, &scissor);

            std::vector<VkDescriptorSet> descriptor_sets = {m_camera_set};

            vkCmdBindDescriptorSets(command_buffer->handle(),
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    m_geometry_pipeline->layout(),
                                    0,
                                    static_cast<uint32_t>(descriptor_sets.size()),
                                    descriptor_sets.data(),
                                    0,
                                    nullptr);

            for (const auto& mesh : m_model->get_meshes()) {
                const auto& vertex_buffer = std::dynamic_pointer_cast<VulkanVertexBuffer>(mesh->get_vertex_buffer());
                const auto& index_buffer = std::dynamic_pointer_cast<VulkanIndexBuffer>(mesh->get_index_buffer());

                ModelInfoPushConstant constants = {
                    .model = glm::mat4(1.0f),
                    .color = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f),
                };

                vkCmdPushConstants(command_buffer->handle(),
                                   m_geometry_pipeline->layout(),
                                   VK_SHADER_STAGE_VERTEX_BIT,
                                   0,
                                   sizeof(ModelInfoPushConstant),
                                   &constants);

                VulkanRendererAPI::draw_indexed(m_command_buffer, vertex_buffer, index_buffer);
            }

            // Draw lights
            m_flat_color_pipeline->bind(command_buffer);
            vkCmdSetViewport(command_buffer->handle(), 0, 1, &viewport);
            vkCmdSetScissor(command_buffer->handle(), 0, 1, &scissor);

            vkCmdBindDescriptorSets(command_buffer->handle(),
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    m_flat_color_pipeline->layout(),
                                    0,
                                    1,
                                    &m_camera_set,
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
                const auto vertex_buffer = std::dynamic_pointer_cast<VulkanVertexBuffer>(mesh->get_vertex_buffer());
                const auto index_buffer = std::dynamic_pointer_cast<VulkanIndexBuffer>(mesh->get_index_buffer());

                VulkanRendererAPI::draw_indexed(m_command_buffer, vertex_buffer, index_buffer);
            }

            m_geometry_pass->end(command_buffer);
        }

        // Lighting pass
        // ==========================
        {
            m_lighting_pass->begin(command_buffer, swapchain_framebuffer);

            // Draw quad
            m_lighting_pipeline->bind(command_buffer);
            vkCmdSetViewport(command_buffer->handle(), 0, 1, &viewport);
            vkCmdSetScissor(command_buffer->handle(), 0, 1, &scissor);

            std::vector<VkDescriptorSet> descriptor_sets = {m_camera_set, m_lighting_fragment_set};

            vkCmdBindDescriptorSets(command_buffer->handle(),
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    m_lighting_pipeline->layout(),
                                    0,
                                    static_cast<uint32_t>(descriptor_sets.size()),
                                    descriptor_sets.data(),
                                    0,
                                    nullptr);

            VulkanRendererAPI::draw_indexed(m_command_buffer, m_quad_vertex, m_quad_index);

            m_lighting_pass->end(command_buffer);
        }
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

void DeferredRenderer::on_event(Event& event) {
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

void DeferredRenderer::update_light_info() {
    light_info.count = 5;

    light_info.positions[0] = glm::vec4(0.0f, 0.0f, 2.0f, 0.0f);
    light_info.colors[0] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

    light_info.positions[1] = glm::vec4(2.0f, 0.0f, 0.0f, 0.0f);
    light_info.colors[1] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

    light_info.positions[2] = glm::vec4(-2.0f, 1.0f, 0.0f, 0.0f);
    light_info.colors[2] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

    light_info.positions[3] = glm::vec4(-1.0f, -1.0f, -1.0f, 0.0f);
    light_info.colors[3] = glm::vec4(0.2f, 0.5f, 0.3f, 1.0f);

    light_info.positions[4] = glm::vec4(-1.0f, -2.0f, 1.0f, 0.0);
    light_info.colors[4] = glm::vec4(0.1f, 0.2f, 0.3f, 1.0f);

    m_lights_ubo->update(light_info);
}

} // namespace Phos
