#pragma once

#include "imgui_panel.h"

namespace Phos {

// Forward declarations
class ISceneRenderer;
class Window;

} // namespace Phos

class ViewportPanel : public IImGuiPanel {
  public:
    ViewportPanel(std::string name, std::shared_ptr<Phos::ISceneRenderer> renderer);
    ~ViewportPanel() override = default;

    void on_imgui_render() override;
    void set_viewport_resized_callback(std::function<void(uint32_t, uint32_t)> func);

  private:
    std::string m_name;
    std::shared_ptr<Phos::ISceneRenderer> m_renderer;

    uint32_t m_width, m_height;
    ImTextureID m_texture_id;

    std::function<void(uint32_t, uint32_t)> m_viewport_resized_callback;
};
