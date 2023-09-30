#pragma once

#include "core.h"

#include "imgui_panel.h"
#include "assets_panel.h"

namespace Phos {

// Forward declarations
class EditorAssetManager;
class Material;

} // namespace Phos

class AssetInspectorPanel : public IImGuiPanel {
  public:
    AssetInspectorPanel(std::string name, std::shared_ptr<Phos::EditorAssetManager> asset_manager);
    ~AssetInspectorPanel() override = default;

    void on_imgui_render() override;

    void set_selected_asset(std::optional<EditorAsset> asset);

  private:
    std::string m_name;
    std::optional<EditorAsset> m_selected_asset;

    std::shared_ptr<Phos::EditorAssetManager> m_asset_manager;
};
