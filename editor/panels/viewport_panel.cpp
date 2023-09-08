#include "viewport_panel.h"

#include <utility>

#include "imgui/imgui_impl.h"

#include "scene/scene_renderer.h"
#include "scene/scene.h"
#include "scene/entity.h"

#include "input/input.h"
#include "input/events.h"

#include "renderer/camera.h"
#include "renderer/backend/texture.h"
#include "renderer/backend/image.h"

ViewportPanel::ViewportPanel(std::string name,
                             std::shared_ptr<Phos::ISceneRenderer> renderer,
                             std::shared_ptr<Phos::Scene> scene,
                             std::shared_ptr<EditorStateManager> state_manager)
      : m_name(std::move(name)), m_renderer(std::move(renderer)), m_scene(std::move(scene)),
        m_state_manager(std::move(state_manager)) {
    const auto& output_texture = m_renderer->output_texture();
    m_texture_id = ImGuiImpl::add_texture(output_texture);

    m_width = output_texture->get_image()->width();
    m_height = output_texture->get_image()->height();

    const auto aspect_ratio = m_width / m_height;
    m_editor_camera = std::make_shared<Phos::PerspectiveCamera>(glm::radians(90.0f), aspect_ratio, 0.001f, 40.0f);
    m_editor_camera->set_position({0.0f, 3.0f, 7.0f});
    m_editor_camera->rotate(glm::vec2(0.0f, glm::radians(30.0f)));
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
    if (!ImGui::DockBuilderGetCentralNode(dockspace_id)->IsFocused || m_state_manager->state != EditorState::Editing)
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

        m_editor_camera->rotate({x_rotation, y_rotation});
    }

    m_mouse_pos = glm::vec2(x, y);
}

void ViewportPanel::on_key_pressed(Phos::KeyPressedEvent& key_pressed, uint32_t dockspace_id) {
    if (!ImGui::DockBuilderGetCentralNode(dockspace_id)->IsFocused || m_state_manager->state != EditorState::Editing)
        return;

    glm::vec3 new_pos = m_editor_camera->non_rotated_position();

    if (key_pressed.get_key() == Phos::Key::W) {
        new_pos.z -= 1;
    } else if (key_pressed.get_key() == Phos::Key::S) {
        new_pos.z += 1;
    } else if (key_pressed.get_key() == Phos::Key::A) {
        new_pos.x -= 1;
    } else if (key_pressed.get_key() == Phos::Key::D) {
        new_pos.x += 1;
    }

    m_editor_camera->set_position(new_pos);
}

std::shared_ptr<Phos::Camera> ViewportPanel::get_camera() const {
    if (m_state_manager->state == EditorState::Editing) {
        return m_editor_camera;
    } else if (m_state_manager->state == EditorState::Playing) {
        auto camera_entities = m_scene->get_entities_with<Phos::CameraComponent>();

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
        }

        scene_camera->set_position(camera_transform.position);
        scene_camera->rotate(camera_transform.rotation);

        return scene_camera;
    }

    return nullptr;
}
