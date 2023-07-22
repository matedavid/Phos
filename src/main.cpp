#include <memory>

#include "core/entry_point.h"
#include "core/layer.h"
#include "core/application.h"

#include "input/input.h"

#include "managers/shader_manager.h"

#include "scene/scene.h"
#include "scene/entity.h"
// #include "scene/model_loader.h"

#include "asset/asset_manager.h"
#include "asset/model_asset.h"

#include "renderer/camera.h"
#include "renderer/mesh.h"
#include "renderer/deferred_renderer.h"
#include "renderer/backend/material.h"
#include "renderer/backend/renderer.h"

constexpr uint32_t WIDTH = 1280;
constexpr uint32_t HEIGHT = 960;

class SandboxLayer : public Phos::Layer {
  public:
    SandboxLayer() {
        m_scene = std::make_shared<Phos::Scene>("Sandbox Scene");
        m_scene_renderer = std::make_shared<Phos::DeferredRenderer>(m_scene);

        const auto pack = std::make_shared<Phos::AssetPack>("../assets/asset_pack.psap");
        m_asset_manager = std::make_shared<Phos::AssetManager>(pack);

        // Camera
        const auto aspect_ratio = WIDTH / HEIGHT;
        m_camera = std::make_shared<Phos::PerspectiveCamera>(glm::radians(90.0f), aspect_ratio, 0.001f, 40.0f);
        m_camera->set_position({0.0f, 0.0f, 4.0f});

        m_scene->set_camera(m_camera);

        //
        // Create scene
        //
        {
            // const auto light_mesh = Phos::ModelLoader::load_single_mesh("../assets/cube.fbx");
            const auto light_mesh = m_asset_manager->load<Phos::Mesh>("../assets/models/cube/cube_mesh.psa");
            const auto light_material = Phos::Material::create(
                Phos::Renderer::shader_manager()->get_builtin_shader("PBR.Geometry.Deferred"), "Light");
            light_material->bake();

            auto light1 = m_scene->create_entity();
            light1.get_component<Phos::TransformComponent>().position = glm::vec3(0.0f, 0.0f, 2.0f);
            light1.get_component<Phos::TransformComponent>().scale = glm::vec3(0.25f);
            light1.add_component<Phos::LightComponent>({
                .light_type = Phos::LightComponent::Type::Point,
                .color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
            });
            light1.add_component<Phos::MeshRendererComponent>({
                .mesh = light_mesh,
                .material = light_material,
            });

            auto light2 = m_scene->create_entity();
            light2.get_component<Phos::TransformComponent>().position = glm::vec3(2.0f, 0.0f, 0.0f);
            light2.get_component<Phos::TransformComponent>().scale = glm::vec3(0.25f);
            light2.add_component<Phos::LightComponent>({
                .light_type = Phos::LightComponent::Type::Point,
                .color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
            });
            light2.add_component<Phos::MeshRendererComponent>({
                .mesh = light_mesh,
                .material = light_material,
            });

            auto light3 = m_scene->create_entity();
            light3.get_component<Phos::TransformComponent>().position = glm::vec3(-2.0f, 3.0f, 0.0f);
            light3.get_component<Phos::TransformComponent>().scale = glm::vec3(0.25f);
            light3.add_component<Phos::LightComponent>({
                .light_type = Phos::LightComponent::Type::Point,
                .color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
            });
            light3.add_component<Phos::MeshRendererComponent>({
                .mesh = light_mesh,
                .material = light_material,
            });
        }

        //
        // Load model
        //
        const auto model = m_asset_manager->load<Phos::ModelAsset>("../assets/models/john_117_imported/scene.gltf.psa");
        model->import_into_scene(m_scene);

        // Phos::ModelLoader::load_into_scene("../assets/john_117/scene.gltf", m_scene);
    }
    ~SandboxLayer() override = default;

    void on_update([[maybe_unused]] double ts) override { m_scene_renderer->render(); }

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

  private:
    std::shared_ptr<Phos::Scene> m_scene;
    std::shared_ptr<Phos::ISceneRenderer> m_scene_renderer;
    std::shared_ptr<Phos::AssetManager> m_asset_manager;

    std::shared_ptr<Phos::Camera> m_camera;
    glm::vec2 m_mouse_pos{};
};

Phos::Application* create_application() {
    auto* application = new Phos::Application("Phos Engine", WIDTH, HEIGHT);
    application->push_layer<SandboxLayer>();

    return application;
}
