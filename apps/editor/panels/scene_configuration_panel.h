#pragma once

#include "imgui_panel.h"

#include <functional>

#include "scene/scene_renderer.h"

namespace Phos {

// Forward declarations
class EditorAssetManager;

} // namespace Phos

class SceneConfigurationPanel : public IImGuiPanel {
  public:
    SceneConfigurationPanel(std::string name,
                            Phos::SceneRendererConfig config,
                            std::shared_ptr<Phos::EditorAssetManager> asset_manager);
    ~SceneConfigurationPanel() override = default;

    void on_imgui_render() override;

    void set_scene_config_updated_callback(std::function<void(Phos::SceneRendererConfig)> func) {
        m_scene_config_updated_callback = std::move(func);
    }

  private:
    std::string m_name;
    Phos::SceneRendererConfig m_config;
    std::shared_ptr<Phos::EditorAssetManager> m_asset_manager;

    std::function<void(Phos::SceneRendererConfig)> m_scene_config_updated_callback;

  void render_rendering_config();
    void render_bloom_config();
    void render_environment_config();
};
