#pragma once

#include "imgui_panel.h"
#include "editor_state_manager.h"

#include <glm/glm.hpp>

namespace Phos {

// Forward declarations
class ISceneRenderer;
class Window;
class PerspectiveCamera;
class Camera;

class MouseMovedEvent;
class KeyPressedEvent;

} // namespace Phos

// Forward declarations
class EditorSceneManager;

class ViewportPanel : public IImGuiPanel {
  public:
    ViewportPanel(std::string name,
                  std::shared_ptr<Phos::ISceneRenderer> renderer,
                  std::shared_ptr<EditorSceneManager> scene_manager,
                  std::shared_ptr<EditorStateManager> state_manager);
    ~ViewportPanel() override = default;

    void on_imgui_render() override;

    void on_mouse_moved(Phos::MouseMovedEvent& mouse_moved, uint32_t dockspace_id);
    void on_key_pressed(Phos::KeyPressedEvent& key_pressed, uint32_t dockspace_id);

  private:
    std::string m_name;
    std::shared_ptr<Phos::ISceneRenderer> m_renderer;
    std::shared_ptr<EditorSceneManager> m_scene_manager;
    std::shared_ptr<EditorStateManager> m_state_manager;

    std::shared_ptr<Phos::PerspectiveCamera> m_editor_camera;
    glm::vec2 m_mouse_pos{};

    uint32_t m_width, m_height;
    ImTextureID m_texture_id;

    [[nodiscard]] std::shared_ptr<Phos::Camera> get_camera() const;
};
