#include <ranges>
#include <fstream>

#include <imgui.h>
#include <imgui_internal.h>

#include "imgui/imgui_impl.h"

#include "core/entry_point.h"
#include "core/layer.h"
#include "core/application.h"
#include "core/window.h"
#include "core/project.h"

#include "panels/viewport_panel.h"
#include "panels/entity_hierarchy_panel.h"
#include "panels/components_panel.h"
#include "panels/content_browser_panel.h"
#include "panels/asset_inspector_panel.h"

#include "asset/editor_asset_manager.h"

#include "scene/scene_renderer.h"
#include "scene/scene_serializer.h"

#include "renderer/mesh.h"
#include "renderer/deferred_renderer.h"

#include "renderer/backend/material.h"
#include "renderer/backend/shader.h"
#include "renderer/backend/texture.h"

#include "editor_state_manager.h"

constexpr uint32_t WIDTH = 1280;
constexpr uint32_t HEIGHT = 960;

class EditorLayer : public Phos::Layer {
  public:
    EditorLayer() {
        m_project = Phos::Project::open("../projects/project1/project1.psproj");

        m_renderer = std::make_shared<Phos::DeferredRenderer>(m_project->scene());

        // Initialize ImGui backend
        ImGuiImpl::initialize(Phos::Application::instance()->get_window());

        // Editor State Manager
        m_state_manager = std::make_shared<EditorStateManager>();

        const auto editor_asset_manager =
            std::dynamic_pointer_cast<Phos::EditorAssetManager>(m_project->asset_manager());
        PS_ASSERT(editor_asset_manager != nullptr, "Project Asset Manager must be of type EditorAssetManager")

        // Panels
        m_viewport_panel = std::make_unique<ViewportPanel>("Viewport", m_renderer, m_project->scene(), m_state_manager);
        m_entity_panel = std::make_unique<EntityHierarchyPanel>("Entities", m_project->scene());
        m_components_panel = std::make_unique<ComponentsPanel>("Components", m_project->scene(), editor_asset_manager);
        m_content_browser_panel = std::make_unique<ContentBrowserPanel>("Content", editor_asset_manager);
        m_asset_inspector_panel = std::make_unique<AssetInspectorPanel>("Inspector", editor_asset_manager);
    }

    ~EditorLayer() override { ImGuiImpl::shutdown(); }

    void on_update([[maybe_unused]] double ts) override {
        ImGuiImpl::new_frame();
        ImGui::NewFrame();

        const auto viewport = ImGui::GetMainViewport();

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        window_flags |= ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_MenuBar;

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

            auto dock_id_right =
                ImGui::DockBuilderSplitNode(m_dockspace_id, ImGuiDir_Right, 0.30f, nullptr, &m_dockspace_id);

            auto dock_id_viewport_up =
                ImGui::DockBuilderSplitNode(m_dockspace_id, ImGuiDir_Up, 0.04f, nullptr, &m_dockspace_id);

            // we now dock our windows into the docking node we made above
            ImGui::DockBuilderDockWindow("Content", dock_id_down);
            ImGui::DockBuilderDockWindow("Entities", dock_id_left);
            ImGui::DockBuilderDockWindow("Components", dock_id_left_down);

            ImGui::DockBuilderDockWindow("Inspector", dock_id_right);
            ImGui::DockBuilderDockWindow("Configuration", dock_id_right);

            ImGui::DockBuilderDockWindow("ViewportControls", dock_id_viewport_up);
            ImGui::DockBuilderDockWindow("Viewport", m_dockspace_id);
            ImGui::DockBuilderFinish(m_dockspace_id);
        }

        ImGui::End();

        // Menu Bar
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open Project...", "Ctrl+O"))
                    open_project();

                if (ImGui::MenuItem("Save Project", "Ctrl+S"))
                    save_project();

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

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
        // Asset Browser
        //
        m_content_browser_panel->on_imgui_render();

        //
        // Inspector Panel
        //
        m_asset_inspector_panel->set_selected_asset(m_content_browser_panel->get_selected_asset());
        m_asset_inspector_panel->on_imgui_render();

        //
        // Configuration Panel
        //
        ImGui::Begin("Configuration");

        ImGui::Text("Configuration");

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
    std::shared_ptr<Phos::Project> m_project;
    std::shared_ptr<Phos::ISceneRenderer> m_renderer;

    std::unique_ptr<ViewportPanel> m_viewport_panel;
    std::unique_ptr<EntityHierarchyPanel> m_entity_panel;
    std::unique_ptr<ComponentsPanel> m_components_panel;
    std::unique_ptr<ContentBrowserPanel> m_content_browser_panel;
    std::unique_ptr<AssetInspectorPanel> m_asset_inspector_panel;

    ImGuiID m_dockspace_id{0};

    std::shared_ptr<EditorStateManager> m_state_manager;

    void open_project() {
        // TODO:
        fmt::print("Open new project...\n");
    }

    void save_project() { Phos::SceneSerializer::serialize(m_project->scene(), "../projects/project1/scene.pssc"); }

    void on_mouse_moved(Phos::MouseMovedEvent& mouse_moved) override {
        m_viewport_panel->on_mouse_moved(mouse_moved, m_dockspace_id);
    }

    void on_key_pressed(Phos::KeyPressedEvent& key_pressed) override {
        m_viewport_panel->on_key_pressed(key_pressed, m_dockspace_id);
    }
};

Phos::Application* create_application() {
    auto* application = new Phos::Application("Phos Editor", WIDTH, HEIGHT);
    application->push_layer<EditorLayer>();

    return application;
}
