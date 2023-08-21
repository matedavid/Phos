#include <ranges>
#include <algorithm>

#include "core/entry_point.h"
#include "core/layer.h"
#include "core/application.h"
#include "core/window.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "imgui/imgui_impl.h"

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

        // Image Descriptors
        const auto& output_texture = m_renderer->output_texture();
        m_set = ImGuiImpl::add_texture(output_texture);
    }

    ~EditorLayer() override { ImGuiImpl::shutdown(); }

    void on_update([[maybe_unused]] double ts) override {
        ImGuiImpl::new_frame();
        ImGui::NewFrame();

        const auto viewport = ImGui::GetMainViewport();

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        window_flags |= ImGuiWindowFlags_NoBackground;

        static ImGuiID dockspace_id;
        static bool first_time = true;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace", nullptr, window_flags);
        ImGui::PopStyleVar();
        ImGui::PopStyleVar(2);

        dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

        if (first_time) {
            first_time = false;

            ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
            ImGui::DockBuilderAddNode(
                dockspace_id, ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace); // Add empty node
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

            auto dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, nullptr, &dockspace_id);
            auto dock_id_down = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.20f, nullptr, &dockspace_id);

            // we now dock our windows into the docking node we made above
            ImGui::DockBuilderDockWindow("Down", dock_id_down);
            ImGui::DockBuilderDockWindow("Entity Panel", dock_id_left);
            ImGui::DockBuilderDockWindow("Viewport", dockspace_id);
            ImGui::DockBuilderFinish(dockspace_id);
        }

        ImGui::End();

        //
        // Entity panel
        //
        ImGui::Begin("Entity Panel");

        render_entity_panel();

        ImGui::End();

        //
        // Down panel
        //
        ImGui::Begin("Down");
        ImGui::Text("Hello, down!");
        ImGui::End();

        //
        // Viewport panel
        //
        ImGuiWindowClass window_class;
        window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
        ImGui::SetNextWindowClass(&window_class);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport",
                     nullptr,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus |
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
        ImGui::PopStyleVar();

        static uint32_t last_width = WIDTH;
        static uint32_t last_height = HEIGHT;

        // const auto viewport_node = ImGui::GetWindowDockNode();
        const auto viewport_node = ImGui::DockBuilderGetCentralNode(dockspace_id);

        const auto width = static_cast<uint32_t>(viewport_node->Size.x);
        const auto height = static_cast<uint32_t>(viewport_node->Size.y);

        if (width != last_width || height != last_height) {
            last_width = width;
            last_height = height;

            m_renderer->window_resized(width, height);
            m_camera->set_aspect_ratio(float(width) / float(height));

            // Image Descriptors
            const auto& output_texture = m_renderer->output_texture();
            m_set = ImGuiImpl::add_texture(output_texture);
        }

        m_renderer->render();
        ImGui::Image(m_set, viewport_node->Size);

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
    std::shared_ptr<Phos::AssetManager> m_asset_manager;
    std::shared_ptr<Phos::PerspectiveCamera> m_camera;

    ImTextureID m_set;

    void render_entity_panel() {
        std::vector<Phos::Entity> parent_entities;
        std::ranges::copy_if(m_scene->get_entities_with<Phos::RelationshipComponent>(),
                             std::back_inserter(parent_entities),
                             [](Phos::Entity& entity) {
                                 auto relationship = entity.get_component<Phos::RelationshipComponent>();
                                 return !relationship.parent.has_value();
                             });

        for (const auto& entity : parent_entities) {
            render_entity_panel_r(entity);
        }
    }

    void render_entity_panel_r(const Phos::Entity& entity) {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;

        const auto name = entity.get_component<Phos::NameComponent>().name;
        const auto children = entity.get_component<Phos::RelationshipComponent>().children;
        if (children.empty())
            flags |= ImGuiTreeNodeFlags_Leaf;

        if (ImGui::TreeNodeEx(name.c_str(), flags)) {
            for (const auto& child_uuid : children) {
                const auto child = m_scene->get_entity_with_uuid(child_uuid);
                render_entity_panel_r(child);
            }

            ImGui::TreePop();
        }
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
