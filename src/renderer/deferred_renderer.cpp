#include "deferred_renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "core/window.h"

#include "input/events.h"
#include "input/input.h"

#include "renderer/static_mesh.h"
#include "renderer/camera.h"
#include "renderer/light.h"

#include "renderer/backend/renderer.h"
#include "renderer/backend/command_buffer.h"
#include "renderer/backend/texture.h"
#include "renderer/backend/image.h"
#include "renderer/backend/framebuffer.h"
#include "renderer/backend/shader.h"
#include "renderer/backend/graphics_pipeline.h"
#include "renderer/backend/render_pass.h"
#include "renderer/backend/buffers.h"

namespace Phos {

DeferredRenderer::DeferredRenderer() {
    Renderer::config().window->add_event_callback_func([&](Event& event) { on_event(event); });

    m_command_buffer = CommandBuffer::create();

    // Uniform buffers
    m_camera_ubo = UniformBuffer::create<CameraUniformBuffer>();

    const auto width = Renderer::config().window->get_width();
    const auto height = Renderer::config().window->get_height();

    // Geometry pass
    {
        m_position_texture = Texture::create(width, height);
        m_normal_texture = Texture::create(width, height);
        m_color_specular_texture = Texture::create(width, height);

        const Image::Description depth_image_description = {
            .width = width,
            .height = height,
            .type = Image::Type::Image2D,
            .format = Image::Format::D32_SFLOAT,
            .transfer = false,
            .attachment = true,
        };
        const auto depth_image = Image::create(depth_image_description);

        const auto position_attachment = Framebuffer::Attachment{
            .image = m_position_texture->get_image(),
            .load_operation = LoadOperation::Clear,
            .store_operation = StoreOperation::Store,
            .clear_value = glm::vec3(0.0f),
        };
        const auto normal_attachment = Framebuffer::Attachment{
            .image = m_normal_texture->get_image(),
            .load_operation = LoadOperation::Clear,
            .store_operation = StoreOperation::Store,
            .clear_value = glm::vec3(0.0f),
        };
        const auto color_specular_attachment = Framebuffer::Attachment{
            .image = m_color_specular_texture->get_image(),
            .load_operation = LoadOperation::Clear,
            .store_operation = StoreOperation::Store,
            .clear_value = glm::vec3(0.0f),
        };
        const auto depth_attachment = Framebuffer::Attachment{
            .image = depth_image,
            .load_operation = LoadOperation::Clear,
            .store_operation = StoreOperation::DontCare,
            .clear_value = glm::vec3(1.0f),
        };

        m_geometry_framebuffer = Framebuffer::create(Framebuffer::Description{
            .attachments = {position_attachment, normal_attachment, color_specular_attachment, depth_attachment},
        });

        m_geometry_pipeline = GraphicsPipeline::create(GraphicsPipeline::Description{
            .shader =
                Shader::create("../assets/shaders/geometry_vertex.spv", "../assets/shaders/geometry_fragment.spv"),
            .target_framebuffer = m_geometry_framebuffer,
        });

        m_geometry_pass = RenderPass::create(RenderPass::Description{
            .debug_name = "Deferred-Geometry",
            .target_framebuffer = m_geometry_framebuffer,
        });
    }

    // Lighting pass
    {
        m_lighting_pipeline = GraphicsPipeline::create(GraphicsPipeline::Description{
            .shader =
                Shader::create("../assets/shaders/lighting_vertex.spv", "../assets/shaders/lighting_fragment.spv"),
            .target_framebuffer = Renderer::presentation_framebuffer(),
        });

        m_lighting_pipeline->add_input("uPositionMap", m_position_texture);
        m_lighting_pipeline->add_input("uNormalMap", m_normal_texture);
        m_lighting_pipeline->add_input("uColorSpecularMap", m_color_specular_texture);
        PS_ASSERT(m_geometry_pipeline->bake(), "Failed to bake Lighting Pipeline")

        m_lighting_pass = RenderPass::create(RenderPass::Description{
            .debug_name = "Deferred-Lighting",
            .presentation_target = true,
        });
    }

    // Flat color pass
    {
        m_flat_color_pipeline = GraphicsPipeline::create(GraphicsPipeline::Description{
            .shader =
                Shader::create("../assets/shaders/flat_color_vertex.spv", "../assets/shaders/flat_color_fragment.spv"),
            .target_framebuffer = m_geometry_framebuffer,
        });
    }

    // Static Meshes
    m_model = std::make_shared<StaticMesh>("../assets/suzanne.fbx", false);
    m_cube = std::make_shared<StaticMesh>("../assets/cube.fbx", false);

    // Camera
    const auto aspect_ratio = width / height;
    m_camera = std::make_shared<PerspectiveCamera>(glm::radians(90.0f), aspect_ratio, 0.001f, 40.0f);
}

DeferredRenderer::~DeferredRenderer() {
    Renderer::wait_idle();
}

void DeferredRenderer::update() {
    update_light_info();

    const FrameInformation frame_info = {
        .camera = m_camera,
        .lights =
            {
                std::make_shared<PointLight>(m_light_info.positions[0], m_light_info.colors[0]),
                std::make_shared<PointLight>(m_light_info.positions[1], m_light_info.colors[1]),
                std::make_shared<PointLight>(m_light_info.positions[2], m_light_info.colors[2]),
                std::make_shared<PointLight>(m_light_info.positions[3], m_light_info.colors[3]),
                std::make_shared<PointLight>(m_light_info.positions[4], m_light_info.colors[4]),
            },
    };

    Renderer::begin_frame(frame_info);

    m_command_buffer->record([&]() {
        // Geometry pass
        // =============
        {
            // m_geometry_pass->begin(m_command_buffer);
            Renderer::begin_render_pass(m_command_buffer, m_geometry_pass);

            // Draw model
            Renderer::bind_graphics_pipeline(m_command_buffer, m_geometry_pipeline);

            auto constants = ModelInfoPushConstant{
                .model = glm::mat4(1.0f),
                .color = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f),
            };

            Renderer::bind_push_constant(m_command_buffer, m_geometry_pipeline, constants);

            Renderer::submit_static_mesh(m_command_buffer, m_model);

            // Draw lights
            Renderer::bind_graphics_pipeline(m_command_buffer, m_flat_color_pipeline);

            for (uint32_t i = 0; i < m_light_info.count; ++i) {
                const glm::vec3 position = glm::vec3(m_light_info.positions[i]);
                const glm::vec4 color = m_light_info.colors[i];

                glm::mat4 model{1.0f};
                model = glm::translate(model, position);
                model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

                constants = {
                    .model = model,
                    .color = color,
                };

                Renderer::bind_push_constant(m_command_buffer, m_flat_color_pipeline, constants);

                Renderer::submit_static_mesh(m_command_buffer, m_cube);
            }

            Renderer::end_render_pass(m_command_buffer, m_geometry_pass);
        }

        // Lighting pass
        // ==========================
        {
            Renderer::begin_render_pass(m_command_buffer, m_lighting_pass, true);

            // Draw quad
            Renderer::bind_graphics_pipeline(m_command_buffer, m_lighting_pipeline);
            Renderer::draw_screen_quad(m_command_buffer);

            Renderer::end_render_pass(m_command_buffer, m_lighting_pass);
        }
    });

    // Submit command buffer
    Renderer::submit_command_buffer(m_command_buffer);

    Renderer::end_frame();
}

void DeferredRenderer::update_light_info() {
    m_light_info.count = 5;

    m_light_info.positions[0] = glm::vec4(0.0f, 0.0f, 2.0f, 0.0f);
    m_light_info.colors[0] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

    m_light_info.positions[1] = glm::vec4(2.0f, 0.0f, 0.0f, 0.0f);
    m_light_info.colors[1] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

    m_light_info.positions[2] = glm::vec4(-2.0f, 1.0f, 0.0f, 0.0f);
    m_light_info.colors[2] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

    m_light_info.positions[3] = glm::vec4(-1.0f, -1.0f, -1.0f, 0.0f);
    m_light_info.colors[3] = glm::vec4(0.2f, 0.5f, 0.3f, 1.0f);

    m_light_info.positions[4] = glm::vec4(-1.0f, -2.0f, 1.0f, 0.0);
    m_light_info.colors[4] = glm::vec4(0.1f, 0.2f, 0.3f, 1.0f);
}

void DeferredRenderer::on_event(Event& event) {
    if (event.get_type() == EventType::MouseMoved) {
        auto mouse_moved = dynamic_cast<MouseMovedEvent&>(event);

        double x = mouse_moved.get_xpos();
        double y = mouse_moved.get_ypos();

        if (Input::is_mouse_button_pressed(MouseButton::Left)) {
            float x_rotation = 0.0f;
            float y_rotation = 0.0f;

            if (x > m_mouse_pos.x) {
                x_rotation -= 0.03f;
            } else if (x < m_mouse_pos.x) {
                x_rotation += 0.03f;
            }

            if (y > m_mouse_pos.y) {
                y_rotation += 0.03f;
            } else if (y < m_mouse_pos.y) {
                y_rotation -= 0.03f;
            }

            m_camera->rotate({x_rotation, y_rotation});
        }

        m_mouse_pos = glm::vec2(x, y);
    } else if (event.get_type() == EventType::KeyPressed) {
        auto key_pressed = dynamic_cast<KeyPressedEvent&>(event);

        glm::vec3 new_pos = m_camera->non_rotated_position();

        if (key_pressed.get_key() == Key::W) {
            new_pos.z -= 1;
        } else if (key_pressed.get_key() == Key::S) {
            new_pos.z += 1;
        } else if (key_pressed.get_key() == Key::A) {
            new_pos.x -= 1;
        } else if (key_pressed.get_key() == Key::D) {
            new_pos.x += 1;
        }

        m_camera->set_position(new_pos);
    }
}

} // namespace Phos
