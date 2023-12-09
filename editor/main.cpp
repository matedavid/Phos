#include <ranges>
#include <fstream>
#include <utility>

#include <imgui.h>
#include <imgui_internal.h>

#include "file_dialog.h"
#include "imgui/imgui_impl.h"
#include "asset_tools/scene_serializer.h"

#include "core/entry_point.h"
#include "core/layer.h"
#include "core/application.h"
#include "core/window.h"
#include "core/project.h"

#include "panels/viewport_panel.h"
#include "panels/entity_hierarchy_panel.h"
#include "panels/content_browser_panel.h"
#include "panels/asset_inspector_panel.h"
#include "panels/scene_configuration_panel.h"
#include "panels/functional/entity_components_renderer.h"

#include "asset/editor_asset_manager.h"

#include "scene/scene_renderer.h"

#include "renderer/deferred_renderer.h"

#include "scripting/scripting_system.h"
#include "scripting/class_handle.h"

#include "editor_state_manager.h"
#include "asset_watcher.h"
#include "editor_scene_manager.h"

constexpr uint32_t WIDTH = 1280;
constexpr uint32_t HEIGHT = 960;

class EditorLayer : public Phos::Layer {
  public:
    EditorLayer() {
        // Initialize ImGui backend
        ImGuiImpl::initialize(Phos::Application::instance()->get_window());

        // Open example project
        // open_project("../projects/project1/project1.psproj");
        open_project("../../SpaceInvaders/SpaceInvaders.psproj");
    }

    ~EditorLayer() override { ImGuiImpl::shutdown(); }

    void on_update([[maybe_unused]] double ts) override {
        // Logic
        if (EditorStateManager::get_state() == EditorState::Playing)
            m_scripting_system->on_update(ts);

        // Rendering
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
            ImGui::DockBuilderDockWindow("Scene Configuration", dock_id_right);

            ImGui::DockBuilderDockWindow("ViewportControls", dock_id_viewport_up);
            ImGui::DockBuilderDockWindow("Viewport", m_dockspace_id);
            ImGui::DockBuilderFinish(m_dockspace_id);
        }

        ImGui::End();

        // Menu Bar
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open Project...", "Ctrl+O"))
                    open_project_dialog();

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
        ImGui::Begin("Components");

        auto selected_entity = m_entity_panel->get_selected_entity();
        if (selected_entity.has_value()) {
            EntityComponentsRenderer::display(
                *selected_entity,
                m_project->scene(),
                std::dynamic_pointer_cast<Phos::EditorAssetManager>(m_project->asset_manager()));
        }

        ImGui::End();

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
        m_scene_configuration_panel->on_imgui_render();

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
        switch (EditorStateManager::get_state()) {
        case EditorState::Editing:
            control_button_text = "Play";
            break;
        case EditorState::Playing:
            control_button_text = "Stop";
            break;
        }

        const auto window_width = ImGui::GetWindowSize().x;
        const auto text_width = ImGui::CalcTextSize(control_button_text.c_str()).x;

        auto change_state_to = EditorStateManager::get_state();
        ImGui::SetCursorPosX((window_width - text_width) * 0.5f);
        if (ImGui::Button(control_button_text.c_str())) {
            if (EditorStateManager::get_state() == EditorState::Editing)
                change_state_to = EditorState::Playing;
            else if (EditorStateManager::get_state() == EditorState::Playing)
                change_state_to = EditorState::Editing;
        }

        ImGui::End();

        ImGui::Begin("Statistics");

        const double fps = 1.0 / ts;
        const std::string fps_text = "fps = " + std::to_string(fps);
        ImGui::Text("%s", fps_text.c_str());

        ImGui::End();

        // Rendering
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        const bool is_minimized = draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f;
        if (!is_minimized) {
            ImGuiImpl::render_frame(draw_data);
            ImGuiImpl::present_frame();
        }

        // Change state if requested
        if (change_state_to != EditorStateManager::get_state())
            EditorStateManager::set_state(change_state_to);

        // Check if any asset has been modified
        m_asset_watcher->check_asset_modified();
    }

  private:
    std::shared_ptr<Phos::Project> m_project;
    std::shared_ptr<Phos::ISceneRenderer> m_renderer;
    std::shared_ptr<Phos::ScriptingSystem> m_scripting_system;

    std::shared_ptr<EditorSceneManager> m_scene_manager;

    std::unique_ptr<ViewportPanel> m_viewport_panel;
    std::unique_ptr<EntityHierarchyPanel> m_entity_panel;
    std::unique_ptr<ContentBrowserPanel> m_content_browser_panel;
    std::unique_ptr<AssetInspectorPanel> m_asset_inspector_panel;
    std::unique_ptr<SceneConfigurationPanel> m_scene_configuration_panel;

    std::shared_ptr<AssetWatcher> m_asset_watcher;

    ImGuiID m_dockspace_id{0};

    void open_project(const std::filesystem::path& path) {
        m_project = Phos::Project::open(path);
        m_renderer = std::make_shared<Phos::DeferredRenderer>(m_project->scene(), m_project->scene()->config());

        m_scripting_system = std::make_shared<Phos::ScriptingSystem>(m_project);
        check_script_updates();

        m_scene_manager = std::make_shared<EditorSceneManager>(m_project->scene());

        // Editor State Manager
        EditorStateManager::subscribe([&](EditorState prev, EditorState new_) { state_changed(prev, new_); });

        const auto editor_asset_manager =
            std::dynamic_pointer_cast<Phos::EditorAssetManager>(m_project->asset_manager());
        PS_ASSERT(editor_asset_manager != nullptr, "Project Asset Manager must be of type EditorAssetManager")

        m_asset_watcher = std::make_shared<AssetWatcher>(m_project->scene(), m_project, m_renderer, m_scripting_system);

        // Panels
        m_viewport_panel = std::make_unique<ViewportPanel>("Viewport", m_renderer, m_scene_manager);
        m_content_browser_panel =
            std::make_unique<ContentBrowserPanel>("Content", editor_asset_manager, m_asset_watcher);
        m_entity_panel =
            std::make_unique<EntityHierarchyPanel>("Entities", m_scene_manager, m_content_browser_panel.get());
        m_asset_inspector_panel =
            std::make_unique<AssetInspectorPanel>("Inspector", m_project->scene(), editor_asset_manager);
        m_scene_configuration_panel = std::make_unique<SceneConfigurationPanel>(
            "Scene Configuration", m_project->scene()->config(), editor_asset_manager);

        m_scene_configuration_panel->set_scene_config_updated_callback([&](Phos::SceneRendererConfig config) {
            m_project->scene()->config() = std::move(config);
            m_renderer->change_config(m_project->scene()->config());
        });
    }

    void state_changed(EditorState prev, EditorState new_) {
        m_entity_panel->clear_selected_entity();

        if (prev == EditorState::Editing && new_ == EditorState::Playing) {
            m_scene_manager->running_changed(true);
            m_scripting_system->start(m_scene_manager->active_scene());
            m_renderer->set_scene(m_scene_manager->active_scene());
        } else if (prev == EditorState::Playing && new_ == EditorState::Editing) {
            m_scene_manager->running_changed(false);
            m_scripting_system->shutdown();
            m_renderer->set_scene(m_scene_manager->active_scene());
        }
    }

    void open_project_dialog() {
        const auto path = FileDialog::open_file_dialog("psproj");
        if (path.has_value() && std::filesystem::exists(*path)) {
            open_project(*path);
        }
    }

    void save_project() {
        const auto scene_path = m_project->path() / m_project->scene()->asset_name;
        SceneSerializer::serialize(m_project->scene(), scene_path);
    }

    void check_script_updates() const {
        // TODO: Also present in AssetWatcher and AssetInspectorPanel, think moving into a separate function
        for (const auto& entity : m_project->scene()->get_entities_with<Phos::ScriptComponent>()) {
            auto& sc = entity.get_component<Phos::ScriptComponent>();
            const auto& handle = m_project->asset_manager()->load_by_id_type<Phos::ClassHandle>(sc.script);

            // Remove fields that do no longer exist in class
            for (const auto& name : sc.field_values | std::views::keys) {
                if (!handle->get_field(name).has_value()) {
                    sc.field_values.erase(name);
                }
            }

            // Add fields present in class and not present in component
            for (const auto& [name, _, type] : handle->get_all_fields()) {
                if (!sc.field_values.contains(name)) {
                    sc.field_values.insert({name, Phos::ClassField::get_default_value(type)});
                }
            }
        }
    }

    void on_mouse_moved(Phos::MouseMovedEvent& mouse_moved) override {
        m_viewport_panel->on_mouse_moved(mouse_moved, m_dockspace_id);
    }

    void on_mouse_scrolled(Phos::MouseScrolledEvent& mouse_scrolled) override {
        m_viewport_panel->on_mouse_scrolled(mouse_scrolled, m_dockspace_id);
    }
};

Phos::Application* create_application() {
    auto* application = new Phos::Application("Phos Editor", WIDTH, HEIGHT);
    application->push_layer<EditorLayer>();

    return application;
}
