#include "viewport_panel.h"

#include <utility>
#include <algorithm>

#include "imgui/imgui_impl.h"
#include "editor_scene_manager.h"
#include "editor_state_manager.h"

#include "scene/scene_renderer.h"
#include "scene/scene.h"

#include "input/input.h"
#include "input/events.h"

#include "renderer/camera.h"
#include "renderer/backend/texture.h"
#include "renderer/backend/image.h"

ViewportPanel::ViewportPanel(std::string name,
                             std::shared_ptr<Phos::ISceneRenderer> renderer,
                             std::shared_ptr<EditorSceneManager> scene_manager)
      : m_name(std::move(name)), m_renderer(std::move(renderer)), m_scene_manager(std::move(scene_manager)) {
    const auto& output_texture = m_renderer->output_texture();
    m_texture_id = ImGuiImpl::add_texture(output_texture);

    m_width = output_texture->get_image()->width();
    m_height = output_texture->get_image()->height();

    const auto aspect_ratio = m_width / m_height;
    m_editor_camera = std::make_shared<Phos::PerspectiveCamera>(glm::radians(90.0f), aspect_ratio, 0.001f, 40.0f);
}

void ViewportPanel::on_imgui_render() {
    ImGuiWindowClass window_class;
    window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
    ImGui::SetNextWindowClass(&window_class);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin(m_name.c_str(),
                 nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleVar();

    const auto viewport_node = ImGui::GetWindowDockNode();
    // const auto viewport_node = ImGui::DockBuilderGetCentralNode(dockspace_id);

    const auto width = static_cast<uint32_t>(viewport_node->Size.x);
    const auto height = static_cast<uint32_t>(viewport_node->Size.y);

    if (width != m_width || height != m_height) {
        m_width = width;
        m_height = height;

        m_renderer->window_resized(m_width, m_height);
        m_editor_camera->set_aspect_ratio((float)m_width / (float)m_height);

        const auto& output_texture = m_renderer->output_texture();
        m_texture_id = ImGuiImpl::add_texture(output_texture);
    }

    // Render scene
    const auto camera = get_camera();
    if (camera != nullptr)
        m_renderer->render(camera);

    ImGui::Image(m_texture_id, viewport_node->Size);

    ImGui::End();
}

void ViewportPanel::on_mouse_moved(Phos::MouseMovedEvent& mouse_moved, uint32_t dockspace_id) {
    if (!ImGui::DockBuilderGetCentralNode(dockspace_id)->IsFocused ||
        EditorStateManager::get_state() != EditorState::Editing)
        return;

    const glm::vec2 new_pos = {mouse_moved.get_xpos(), mouse_moved.get_ypos()};

    if (Phos::Input::is_mouse_button_pressed(Phos::MouseButton::Right)) {
        constexpr float ROTATION_VELOCITY = 0.03f;

        const auto offset = (new_pos - m_mouse_pos) * ROTATION_VELOCITY;

        const glm::quat rotation = [&]() {
            const auto new_rotation = glm::angleAxis(-offset.x, glm::vec3(0.0f, 1.0f, 0.0f)) *
                                      glm::angleAxis(-offset.y, glm::vec3(1.0f, 0.0f, 0.0f));

            auto rotation = new_rotation * m_editor_camera->rotation();
            rotation.z = 0.0f; // Prevent rotation on z-axis

            return rotation;
        }();

        m_editor_camera->set_rotation(rotation);
    } else if (Phos::Input::is_mouse_button_pressed(Phos::MouseButton::Middle)) {
        auto non_rotated_pos = m_editor_camera->non_rotated_position();

        constexpr float MOVEMENT = 0.02f;

        const auto offset = (new_pos - m_mouse_pos) * MOVEMENT;

        const auto inverse_view_matrix = glm::mat3(glm::inverse(m_editor_camera->view_matrix()));
        m_editor_camera->set_position(m_editor_camera->position() - inverse_view_matrix * glm::vec3(offset, 0.0f));
    }

    m_mouse_pos = new_pos;
}

void ViewportPanel::on_key_pressed(Phos::KeyPressedEvent& key_Pressed, uint32_t dockspace_id) {
    if (!ImGui::DockBuilderGetCentralNode(dockspace_id)->IsFocused ||
        EditorStateManager::get_state() != EditorState::Editing)
        return;

    constexpr float MOVEMENT_VELOCITY = 0.2f;
    glm::vec3 movement{0.0f};

    if (key_Pressed.get_key() == Phos::Key::W) {
        movement += glm::vec3(0.0f, 0.0f, -1.0f) * MOVEMENT_VELOCITY;
    } else if (key_Pressed.get_key() == Phos::Key::S) {
        movement += glm::vec3(0.0f, 0.0f, 1.0f) * MOVEMENT_VELOCITY;
    } else if (key_Pressed.get_key() == Phos::Key::A) {
        movement += glm::vec3(-1.0f, 0.0f, 0.0f) * MOVEMENT_VELOCITY;
    } else if (key_Pressed.get_key() == Phos::Key::D) {
        movement += glm::vec3(1.0f, 0.0f, 0.0f) * MOVEMENT_VELOCITY;
    }

    const auto inverse_view_matrix = glm::mat3(glm::inverse(m_editor_camera->view_matrix()));
    m_editor_camera->set_position(m_editor_camera->position() + inverse_view_matrix * movement);
}

std::shared_ptr<Phos::Camera> ViewportPanel::get_camera() const {
    if (EditorStateManager::get_state() == EditorState::Editing) {
        return m_editor_camera;
    } else if (EditorStateManager::get_state() == EditorState::Playing) {
        auto camera_entities = m_scene_manager->active_scene()->get_entities_with<Phos::CameraComponent>();

        if (camera_entities.empty())
            return nullptr;

        std::ranges::sort(camera_entities, [](Phos::Entity& a, Phos::Entity& b) {
            return a.get_component<Phos::CameraComponent>().depth < b.get_component<Phos::CameraComponent>().depth;
        });

        const auto& scene_camera_entity = camera_entities[0];
        const auto& camera_component = scene_camera_entity.get_component<Phos::CameraComponent>();
        const auto& camera_transform = scene_camera_entity.get_component<Phos::TransformComponent>();

        std::shared_ptr<Phos::Camera> scene_camera;
        if (camera_component.type == Phos::Camera::Type::Perspective) {
            scene_camera = std::make_shared<Phos::PerspectiveCamera>(
                camera_component.fov, (float)m_width / (float)m_height, camera_component.znear, camera_component.zfar);
        } else {
            PHOS_LOG_ERROR("OrthographicCamera not implemented");
        }

        scene_camera->set_position(camera_transform.position);
        scene_camera->rotate(camera_transform.rotation);

        return scene_camera;
    }

    return nullptr;
}
