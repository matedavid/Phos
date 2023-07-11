#include "forward_renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "core/window.h"

#include "input/events.h"
#include "input/input.h"

#include "managers/shader_manager.h"

#include "renderer/mesh.h"
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
#include "renderer/backend/material.h"
#include "renderer/backend/cubemap.h"

namespace Phos {

ForwardRenderer::ForwardRenderer() {
    Renderer::config().window->add_event_callback_func([&](Event& event) { on_event(event); });

    m_command_buffer = CommandBuffer::create();

    const auto width = Renderer::config().window->get_width();
    const auto height = Renderer::config().window->get_height();

    // Rendering information
    m_render_pass = RenderPass::create(RenderPass::Description{
        .debug_name = "Presentation Render Pass",
        .presentation_target = true,
    });

    m_pbr_pipeline = GraphicsPipeline::create(GraphicsPipeline::Description{
        .shader = Renderer::shader_manager()->get_builtin_shader("PBR.Forward"),
        .target_framebuffer = Renderer::presentation_framebuffer(),
    });

    const auto faces = Cubemap::Faces{
        .right = "right.jpg",
        .left = "left.jpg",
        .top = "top.jpg",
        .bottom = "bottom.jpg",
        .front = "front.jpg",
        .back = "back.jpg",
    };
    m_skybox = Cubemap::create(faces, "../assets/skybox/");

    m_skybox_pipeline = GraphicsPipeline::create(GraphicsPipeline::Description{
        .shader = Renderer::shader_manager()->get_builtin_shader("Cubemap"),
        .target_framebuffer = Renderer::presentation_framebuffer(),

        .front_face = FrontFace::Clockwise,
        .depth_compare_op = DepthCompareOp::LessEq,
    });

    m_skybox_pipeline->add_input("uSkybox", m_skybox);
    PS_ASSERT(m_skybox_pipeline->bake(), "Failed to bake Cubemap Pipeline")

    // Models
    m_model = std::make_shared<Mesh>("../assets/john_117/scene.gltf", false);
    m_cube = std::make_shared<Mesh>("../assets/cube.fbx", false);

    m_cube_material = Material::create(Renderer::shader_manager()->get_builtin_shader("Cubemap"), "SkyboxMaterial");
    m_cube_material->bake();

    for (auto& submesh : m_cube->get_sub_meshes())
        submesh->set_material(m_cube_material);

    // Camera
    const auto aspect_ratio = width / height;
    m_camera = std::make_shared<PerspectiveCamera>(glm::radians(90.0f), aspect_ratio, 0.001f, 40.0f);
    m_camera->set_position({0.0f, 0.0f, 4.0f});
}

ForwardRenderer::~ForwardRenderer() {
    Renderer::wait_idle();
}

void ForwardRenderer::update() {
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
        Renderer::begin_render_pass(m_command_buffer, m_render_pass, true);

        // Draw models
        Renderer::bind_graphics_pipeline(m_command_buffer, m_pbr_pipeline);

        auto constants = ModelInfoPushConstant{
            .model = glm::mat4{1.0f},
            .color = glm::vec4{1.0f},
        };
        Renderer::bind_push_constant(m_command_buffer, m_pbr_pipeline, constants);
        Renderer::submit_static_mesh(m_command_buffer, m_model);

        // Cubemap
        Renderer::bind_graphics_pipeline(m_command_buffer, m_skybox_pipeline);

        glm::mat4 model{1.0f};
        model = glm::scale(model, glm::vec3(1.0f));

        constants = ModelInfoPushConstant{
            .model = model,
            .color = glm::vec4{1.0f},
        };
        Renderer::bind_push_constant(m_command_buffer, m_pbr_pipeline, constants);
        Renderer::submit_static_mesh(m_command_buffer, m_cube);

        Renderer::end_render_pass(m_command_buffer, m_render_pass);
    });

    Renderer::submit_command_buffer(m_command_buffer);

    Renderer::end_frame();
}

void ForwardRenderer::update_light_info() {
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

void ForwardRenderer::on_event(Event& event) {
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
