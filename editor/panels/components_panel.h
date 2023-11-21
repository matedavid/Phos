#pragma once

#include "imgui_panel.h"

#include "scene/entity.h"

namespace Phos {

// Forward declarations
class EditorAssetManager;

} // namespace Phos

class ComponentsPanel : public IImGuiPanel {
  public:
    ComponentsPanel(std::string name, std::shared_ptr<Phos::EditorAssetManager> asset_manager);
    ~ComponentsPanel() override = default;

    void on_imgui_render() override;
    void set_selected_entity(const Phos::Entity& entity);
    void deselect_entity();

  private:
    std::string m_name;
    std::optional<Phos::Entity> m_selected_entity;
    std::shared_ptr<Phos::EditorAssetManager> m_asset_manager;
};
