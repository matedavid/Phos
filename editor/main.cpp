#include <ranges>
#include <algorithm>

#include "core/entry_point.h"
#include "core/layer.h"
#include "core/application.h"
#include "core/window.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "imgui/imgui_impl.h"

#include "panels/viewport_panel.h"
#include "panels/entity_hierarchy_panel.h"
#include "panels/components_panel.h"

#include "asset/editor_asset_manager.h"
#include "asset/runtime_asset_manager.h"

#include "scene/scene_renderer.h"
#include "scene/scene.h"

#include "managers/shader_manager.h"

#include "renderer/camera.h"
#include "renderer/mesh.h"
#include "renderer/deferred_renderer.h"

#include "renderer/backend/renderer.h"
#include "renderer/backend/material.h"
#include "renderer/backend/shader.h"
#include "renderer/backend/texture.h"

#include "editor_state_manager.h"

constexpr uint32_t WIDTH = 1280;
constexpr uint32_t HEIGHT = 960;

class EditorLayer : public Phos::Layer {
  public:
    EditorLayer() {
        m_asset_manager =
            std::make_shared<Phos::RuntimeAssetManager>(std::make_shared<Phos::AssetPack>("../assets/asset_pack.psap"));
        m_scene = std::make_shared<Phos::Scene>("Example");
        m_renderer = std::make_shared<Phos::DeferredRenderer>(m_scene);

        create_scene();

        // Initialize ImGui backend
        ImGuiImpl::initialize(Phos::Application::instance()->get_window());

        // Editor State Manager
        m_state_manager = std::make_shared<EditorStateManager>();

        // Panels
        m_viewport_panel = std::make_unique<ViewportPanel>("Viewport", m_renderer, m_scene, m_state_manager);
        m_entity_panel = std::make_unique<EntityHierarchyPanel>("Entities", m_scene);
        m_components_panel = std::make_unique<ComponentsPanel>("Components", m_scene);
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

            ImGui::DockBuilderRemoveNode(m_dockspace_id);
            ImGui::DockBuilderAddNode(m_dockspace_id,
                                      ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(m_dockspace_id, viewport->Size);

            auto dock_id_left =
                ImGui::DockBuilderSplitNode(m_dockspace_id, ImGuiDir_Left, 0.20f, nullptr, &m_dockspace_id);
            auto dock_id_left_down =
                ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.40f, nullptr, &dock_id_left);

            auto dock_id_down =
                ImGui::DockBuilderSplitNode(m_dockspace_id, ImGuiDir_Down, 0.20f, nullptr, &m_dockspace_id);

            auto dock_id_viewport_up =
                ImGui::DockBuilderSplitNode(m_dockspace_id, ImGuiDir_Up, 0.04f, nullptr, &m_dockspace_id);

            // we now dock our windows into the docking node we made above
            ImGui::DockBuilderDockWindow("Down", dock_id_down);
            ImGui::DockBuilderDockWindow("Entities", dock_id_left);
            ImGui::DockBuilderDockWindow("Components", dock_id_left_down);
            ImGui::DockBuilderDockWindow("ViewportControls", dock_id_viewport_up);
            ImGui::DockBuilderDockWindow("Viewport", m_dockspace_id);
            ImGui::DockBuilderFinish(m_dockspace_id);
        }

        ImGui::End();

        //
        // Entity Hierarchy
        //
        m_entity_panel->on_imgui_render();

        //
        // Components
        //
        const auto selected_entity = m_entity_panel->get_selected_entity();
        if (selected_entity.has_value())
            m_components_panel->set_selected_entity(selected_entity.value());
        else
            m_components_panel->deselect_entity();

        m_components_panel->on_imgui_render();

        //
        // Down Panel
        //
        ImGui::Begin("Down");
        ImGui::Text("Hello, down!");
        ImGui::End();

        //
        // Viewport
        //
        m_viewport_panel->on_imgui_render();

        //
        // Viewport Controls
        //
        ImGuiWindowClass window_class;
        window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
        ImGui::SetNextWindowClass(&window_class);

        ImGui::Begin("ViewportControls",
                     nullptr,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus |
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);

        std::string control_button_text;
        switch (m_state_manager->state) {
        case EditorState::Editing:
            control_button_text = "Play";
            break;
        case EditorState::Playing:
            control_button_text = "Stop";
            break;
        }

        auto windowWidth = ImGui::GetWindowSize().x;
        auto textWidth = ImGui::CalcTextSize(control_button_text.c_str()).x;

        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        if (ImGui::Button(control_button_text.c_str())) {
            if (m_state_manager->state == EditorState::Editing)
                m_state_manager->state = EditorState::Playing;
            else if (m_state_manager->state == EditorState::Playing)
                m_state_manager->state = EditorState::Editing;
        }

        ImGui::End();

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
    std::shared_ptr<Phos::RuntimeAssetManager> m_asset_manager;

    std::unique_ptr<ViewportPanel> m_viewport_panel;
    std::unique_ptr<EntityHierarchyPanel> m_entity_panel;
    std::unique_ptr<ComponentsPanel> m_components_panel;

    ImGuiID m_dockspace_id{0};

    std::shared_ptr<EditorStateManager> m_state_manager;

    void on_mouse_moved(Phos::MouseMovedEvent& mouse_moved) override {
        m_viewport_panel->on_mouse_moved(mouse_moved, m_dockspace_id);
    }

    void on_key_pressed(Phos::KeyPressedEvent& key_pressed) override {
        m_viewport_panel->on_key_pressed(key_pressed, m_dockspace_id);
    }

    void create_scene() {
        const auto sphere_mesh = m_asset_manager->load_type<Phos::Mesh>("../assets/models/sphere/sphere.psa");
        const auto cube_mesh = m_asset_manager->load_type<Phos::Mesh>("../assets/models/cube/cube.psa");

        // Camera
        auto camera_entity = m_scene->create_entity("Main Camera");
        camera_entity.get_component<Phos::TransformComponent>().position = glm::vec3(0.0f, 5.0f, 5.0f);
        camera_entity.get_component<Phos::TransformComponent>().rotation = glm::vec3(0.0f, glm::radians(20.0f), 0.0f);
        camera_entity.add_component<Phos::CameraComponent>({
            .type = Phos::Camera::Type::Perspective,
            .fov = 90.0f,
            .znear = 0.001f,
            .zfar = 40.0f,
        });

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
                .type = Phos::Light::Type::Point,
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
            .type = Phos::Light::Type::Directional,
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
