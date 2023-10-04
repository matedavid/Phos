#pragma once

#include "core.h"

#include <glm/glm.hpp>

#include "imgui_panel.h"
#include "content_browser_panel.h"

#include "core/uuid.h"

namespace Phos {

// Forward declarations
class EditorAssetManager;
class Material;

} // namespace Phos

// Forward declarations
class EditorMaterialHelper;

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

    bool m_locked = false;

    // Texture type
    std::shared_ptr<Phos::Texture> m_texture;
    ImTextureID m_imgui_texture_id{};

    // Material type
    std::shared_ptr<EditorMaterialHelper> m_material_helper;

    // Render functions
    void render_texture_asset() const;
};
