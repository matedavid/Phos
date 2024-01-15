#include <memory>

#include "core/entry_point.h"
#include "core/layer.h"
#include "core/application.h"
#include "core/project.h"

#include "input/input.h"

#include "managers/shader_manager.h"

#include "scene/scene.h"
#include "scene/entity.h"

#include "asset/asset_manager.h"

#include "renderer/camera.h"
#include "renderer/mesh.h"
#include "renderer/deferred_renderer.h"
#include "renderer/backend/material.h"
#include "renderer/backend/texture.h"
#include "renderer/backend/presenter.h"

constexpr uint32_t WIDTH = 1280;
constexpr uint32_t HEIGHT = 960;

class SandboxLayer : public Phos::Layer {
  public:
    SandboxLayer() {
        m_project = Phos::Project::open("../projects/project1/project1.psproj");

        m_scene_renderer = std::make_shared<Phos::DeferredRenderer>(m_project->scene(), m_project->scene()->config());
        m_presenter = Phos::Presenter::create(m_scene_renderer, Phos::Application::instance()->get_window());

        // Camera
        constexpr auto aspect_ratio = WIDTH / HEIGHT;
        m_camera = std::make_shared<Phos::PerspectiveCamera>(glm::radians(90.0f), aspect_ratio, 0.001f, 40.0f);
        m_camera->set_position({0.0f, 3.0f, 7.0f});
        m_camera->rotate(glm::vec2(0.0f, glm::radians(30.0f)));
    }
    ~SandboxLayer() override = default;

    void on_update([[maybe_unused]] double ts) override {
        m_scene_renderer->render(m_camera);
        m_presenter->present();
    }

    void on_mouse_moved(Phos::MouseMovedEvent& mouse_moved) override {
        double x = mouse_moved.get_xpos();
        double y = mouse_moved.get_ypos();

        if (Phos::Input::is_mouse_button_pressed(Phos::MouseButton::Left)) {
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
    }

    void on_key_pressed(Phos::KeyPressedEvent& key_pressed) override {
        glm::vec3 new_pos = m_camera->non_rotated_position();

        if (key_pressed.get_key() == Phos::Key::W) {
            new_pos.z -= 1;
        } else if (key_pressed.get_key() == Phos::Key::S) {
            new_pos.z += 1;
        } else if (key_pressed.get_key() == Phos::Key::A) {
            new_pos.x -= 1;
        } else if (key_pressed.get_key() == Phos::Key::D) {
            new_pos.x += 1;
        }

        m_camera->set_position(new_pos);
    }

    void on_window_resized(Phos::WindowResizeEvent& window_resized) override {
        const auto [width, height] = window_resized.get_dimensions();

        m_scene_renderer->window_resized(width, height);
        m_presenter->window_resized(width, height);

        const float aspect_ratio = (float)width / (float)height;
        m_camera->set_aspect_ratio(aspect_ratio);
    }

  private:
    std::shared_ptr<Phos::Project> m_project;

    std::shared_ptr<Phos::ISceneRenderer> m_scene_renderer;
    std::shared_ptr<Phos::Presenter> m_presenter;

    std::shared_ptr<Phos::PerspectiveCamera> m_camera;
    glm::vec2 m_mouse_pos{};
};

Phos::Application* create_application() {
    auto* application = new Phos::Application("Phos Engine", WIDTH, HEIGHT);
    application->push_layer<SandboxLayer>();

    return application;
}
