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
    void set_asset_modified_callback(std::function<void(const Phos::UUID&)> func) {
        m_asset_modified_callback = std::move(func);
    }

  private:
    std::string m_name;
    std::optional<EditorAsset> m_selected_asset;

    std::shared_ptr<Phos::EditorAssetManager> m_asset_manager;

    std::function<void(const Phos::UUID&)> m_asset_modified_callback;

    bool m_locked = false;

    // Texture type
    std::shared_ptr<Phos::Texture> m_texture;
    ImTextureID m_imgui_texture_id{};

    // Material type
    std::shared_ptr<EditorMaterialHelper> m_material_helper;

    // Render functions
    void render_texture_asset() const;
    void render_material_asset() const;
};
