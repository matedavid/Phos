#include <ranges>
#include <algorithm>

#include "core/entry_point.h"
#include "core/layer.h"
#include "core/application.h"
#include "core/window.h"

#include "input/input.h"
#include "input/events.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "imgui/imgui_impl.h"

#include "panels/viewport_panel.h"
#include "panels/entity_hierarchy_panel.h"

#include "asset/asset_manager.h"

#include "scene/scene_renderer.h"
#include "scene/scene.h"
#include "scene/entity.h"

#include "managers/shader_manager.h"

#include "renderer/camera.h"
#include "renderer/mesh.h"
#include "renderer/deferred_renderer.h"

#include "renderer/backend/renderer.h"
#include "renderer/backend/material.h"
#include "renderer/backend/shader.h"
#include "renderer/backend/texture.h"

constexpr uint32_t WIDTH = 1280;
constexpr uint32_t HEIGHT = 960;

class EditorLayer : public Phos::Layer {
  public:
    EditorLayer() {
        m_asset_manager =
            std::make_shared<Phos::AssetManager>(std::make_shared<Phos::AssetPack>("../assets/asset_pack.psap"));
        m_scene = std::make_shared<Phos::Scene>("Example");
        m_renderer = std::make_shared<Phos::DeferredRenderer>(m_scene);

        create_scene();

        const auto aspect_ratio = WIDTH / HEIGHT;
        m_camera = std::make_shared<Phos::PerspectiveCamera>(glm::radians(90.0f), aspect_ratio, 0.001f, 40.0f);
        m_camera->set_position({0.0f, 3.0f, 7.0f});
        m_camera->rotate(glm::vec2(0.0f, glm::radians(30.0f)));

        m_scene->set_camera(m_camera);

        // Initialize ImGui backend
        ImGuiImpl::initialize(Phos::Application::instance()->get_window());

        // Panels
        m_viewport_panel = std::make_unique<ViewportPanel>("Viewport", m_renderer);
        m_viewport_panel->set_viewport_resized_callback(
            [&](uint32_t width, uint32_t height) { on_viewport_resized(width, height); });

        m_entity_panel = std::make_unique<EntityHierarchyPanel>("Entities", m_scene);
    }

    ~EditorLayer() override { ImGuiImpl::shutdown(); }

    void on_update([[maybe_unused]] double ts) override {
        ImGuiImpl::new_frame();
        ImGui::NewFrame();

        const auto viewport = ImGui::GetMainViewport();

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        window_flags |= ImGuiWindowFlags_NoBackground;

        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        static bool first_time = true;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace", nullptr, window_flags);
        ImGui::PopStyleVar();
        ImGui::PopStyleVar(2);

        m_dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(m_dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

        if (first_time) {
            first_time = false;

            ImGui::DockBuilderRemoveNode(m_dockspace_id); // Clear out existing layout
            ImGui::DockBuilderAddNode(m_dockspace_id, ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace); // Add empty node
            ImGui::DockBuilderSetNodeSize(m_dockspace_id, viewport->Size);

            auto dock_id_left = ImGui::DockBuilderSplitNode(m_dockspace_id, ImGuiDir_Left, 0.2f, nullptr, &m_dockspace_id);
            auto dock_id_down = ImGui::DockBuilderSplitNode(m_dockspace_id, ImGuiDir_Down, 0.20f, nullptr, &m_dockspace_id);

            // we now dock our windows into the docking node we made above
            ImGui::DockBuilderDockWindow("Down", dock_id_down);
            ImGui::DockBuilderDockWindow("Entities", dock_id_left);
            ImGui::DockBuilderDockWindow("Viewport", m_dockspace_id);
            ImGui::DockBuilderFinish(m_dockspace_id);
        }

        ImGui::End();

        //
        // Entity Hierarchy
        //
        m_entity_panel->on_imgui_render();

        //
        // Down panel
        //
        ImGui::Begin("Down");
        ImGui::Text("Hello, down!");
        ImGui::End();

        //
        // Viewport panel
        //
        m_viewport_panel->on_imgui_render();

        // Rendering
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        if (!is_minimized) {
            ImGuiImpl::render_frame(draw_data);
            ImGuiImpl::present_frame();
        }
    }

  private:
    std::shared_ptr<Phos::ISceneRenderer> m_renderer;
    std::shared_ptr<Phos::Scene> m_scene;
    std::shared_ptr<Phos::AssetManager> m_asset_manager;
    std::shared_ptr<Phos::PerspectiveCamera> m_camera;
    glm::vec2 m_mouse_pos{};

    std::unique_ptr<ViewportPanel> m_viewport_panel;
    std::unique_ptr<EntityHierarchyPanel> m_entity_panel;

    ImGuiID m_dockspace_id;

    void on_viewport_resized(uint32_t width, uint32_t height) {
        m_camera->set_aspect_ratio(float(width) / float(height));
    }

    void on_mouse_moved(Phos::MouseMovedEvent& mouse_moved) override {
        if (!ImGui::DockBuilderGetCentralNode(m_dockspace_id)->IsFocused)
            return;

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
        if (!ImGui::DockBuilderGetCentralNode(m_dockspace_id)->IsFocused)
            return;

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

    void create_scene() {
        const auto sphere_mesh = m_asset_manager->load<Phos::Mesh>("../assets/models/sphere/sphere.psa");
        const auto cube_mesh = m_asset_manager->load<Phos::Mesh>("../assets/models/cube/cube.psa");

        // model
        auto model = m_asset_manager->load<Phos::ModelAsset>("../assets/models/john_117_imported/scene.gltf.psa");
        model->import_into_scene(m_scene);

        // Floor
        const auto floor_material = Phos::Material::create(
            Phos::Renderer::shader_manager()->get_builtin_shader("PBR.Geometry.Deferred"), "Floor Material");

        floor_material->set("uAlbedoMap", Phos::Texture::create("../assets/wood_mat/hardwood-brown-planks-albedo.png"));
        floor_material->set("uMetallicMap",
                            Phos::Texture::create("../assets/wood_mat/hardwood-brown-planks-metallic.png"));
        floor_material->set("uRoughnessMap",
                            Phos::Texture::create("../assets/wood_mat/hardwood-brown-planks-roughness.png"));
        floor_material->set("uAOMap", Phos::Texture::create("../assets/wood_mat/hardwood-brown-planks-ao.png"));
        floor_material->set("uNormalMap",
                            Phos::Texture::create("../assets/wood_mat/hardwood-brown-planks-normal-ogl.png"));

        PS_ASSERT(floor_material->bake(), "Failed to bake floor material")

        const auto light_material = Phos::Material::create(
            Phos::Renderer::shader_manager()->get_builtin_shader("PBR.Geometry.Deferred"), "Light Material");
        PS_ASSERT(light_material->bake(), "Failed to bake light material")

        auto floor_entity = m_scene->create_entity("Floor");
        floor_entity.get_component<Phos::TransformComponent>().scale = glm::vec3(10.0f, 0.25f, 10.0f);
        floor_entity.get_component<Phos::TransformComponent>().position = glm::vec3(0.0f, -0.25f, 0.0f);
        floor_entity.add_component<Phos::MeshRendererComponent>({
            .mesh = cube_mesh,
            .material = floor_material,
        });

        auto wall_entity = m_scene->create_entity("Wall");
        wall_entity.get_component<Phos::TransformComponent>().scale = glm::vec3(10.0f, 0.25f, 7.0f);
        wall_entity.get_component<Phos::TransformComponent>().position = glm::vec3(0.0f, 6.5f, -10.0f);
        wall_entity.get_component<Phos::TransformComponent>().rotation = glm::vec3(glm::radians(90.0f), 0.0f, 0.0f);
        wall_entity.add_component<Phos::MeshRendererComponent>({
            .mesh = cube_mesh,
            .material = floor_material,
        });

        // Spheres
        const uint32_t number = 4;
        for (uint32_t metallic_idx = 0; metallic_idx < number; ++metallic_idx) {
            for (uint32_t roughness_idx = 0; roughness_idx < number; ++roughness_idx) {
                const auto sphere_material = Phos::Material::create(
                    Phos::Renderer::shader_manager()->get_builtin_shader("PBR.Geometry.Deferred"), "Sphere Material");

                const float metallic = (float)(metallic_idx + 1) / (float)number;
                const float roughness = (float)(roughness_idx + 1) / (float)number;

                sphere_material->set("uMaterialInfo.albedo", glm::vec3(0.8f, 0.05f, 0.05f));
                sphere_material->set("uMaterialInfo.metallic", metallic);
                sphere_material->set("uMaterialInfo.roughness", roughness);
                sphere_material->set("uMaterialInfo.ao", 1.0f);

                PS_ASSERT(sphere_material->bake(), "Failed to bake sphere material {} {}", metallic_idx, roughness_idx)

                auto entity_name = "Sphere_" + std::to_string(metallic_idx) + "_" + std::to_string(roughness_idx);
                auto sphere_entity = m_scene->create_entity(entity_name);

                sphere_entity.get_component<Phos::TransformComponent>().position =
                    glm::vec3(metallic_idx * 2, roughness_idx * 2 + 1, -2.0f);
                sphere_entity.get_component<Phos::TransformComponent>().scale = glm::vec3(0.6f);
                sphere_entity.get_component<Phos::TransformComponent>().rotation =
                    glm::vec3(glm::radians(0.0f), 0.0f, 0.0f);
                sphere_entity.add_component<Phos::MeshRendererComponent>({
                    .mesh = sphere_mesh,
                    .material = sphere_material,
                });
            }
        }

        // Lights
        const std::vector<glm::vec3> light_positions = {
            glm::vec3(2.0f, 3.0f, 1.5f),
            glm::vec3(4.0f, 2.0f, 1.5f),
            glm::vec3(5.0f, 6.0f, 1.5f),
            glm::vec3(5.0f, 1.0f, 1.5f),
        };

        for (const auto& light_pos : light_positions) {
            auto light_entity = m_scene->create_entity("PointLight");
            light_entity.get_component<Phos::TransformComponent>().position = light_pos;
            light_entity.get_component<Phos::TransformComponent>().scale = glm::vec3(0.15f);

            light_entity.add_component<Phos::LightComponent>({
                .light_type = Phos::Light::Type::Point,
                .color = glm::vec4(1.0f),
            });

            light_entity.add_component<Phos::MeshRendererComponent>({
                .mesh = cube_mesh,
                .material = light_material,
            });
        }

        auto directional_light_entity = m_scene->create_entity("DirectionalLight");
        directional_light_entity.get_component<Phos::TransformComponent>().position = glm::vec3(2.0f, 17.0f, 9.0f);
        directional_light_entity.get_component<Phos::TransformComponent>().rotation =
            glm::vec3(glm::radians(40.0f), glm::radians(180.0f), glm::radians(0.0f));
        directional_light_entity.get_component<Phos::TransformComponent>().scale = glm::vec3(0.15f);

        directional_light_entity.add_component<Phos::MeshRendererComponent>({
            .mesh = cube_mesh,
            .material = light_material,
        });

        directional_light_entity.add_component<Phos::LightComponent>({
            .light_type = Phos::Light::Type::Directional,
            .color = glm::vec4(1.0f),

            .shadow_type = Phos::Light::ShadowType::Hard,
        });
    }
};

Phos::Application* create_application() {
    auto* application = new Phos::Application("Phos Editor", WIDTH, HEIGHT);
    application->push_layer<EditorLayer>();

    return application;
}
