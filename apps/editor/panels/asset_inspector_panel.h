#pragma once

#include "imgui_panel.h"
#include "content_browser_panel.h"

#include "scene/entity.h"

namespace Phos {

// Forward declarations
class EditorAssetManager;
class Material;
class Scene;

} // namespace Phos

// Forward declarations
class EditorMaterialHelper;
class EditorCubemapHelper;
class EditorPrefabHelper;

class AssetInspectorPanel : public IImGuiPanel {
  public:
    AssetInspectorPanel(std::string name, std::shared_ptr<Phos::Scene> scene, std::shared_ptr<Phos::EditorAssetManager> asset_manager);
    ~AssetInspectorPanel() override = default;

    void on_imgui_render() override;

    void set_selected_asset(std::optional<EditorAsset> asset);

  private:
    std::string m_name;
    std::optional<EditorAsset> m_selected_asset;

    std::shared_ptr<Phos::Scene> m_scene;
    std::shared_ptr<Phos::EditorAssetManager> m_asset_manager;

    bool m_locked = false;

    // Texture type
    std::shared_ptr<Phos::Texture> m_texture;
    ImTextureID m_imgui_texture_id{};

    // Material type
    std::shared_ptr<EditorMaterialHelper> m_material_helper;

    // Cubemap type
    std::shared_ptr<EditorCubemapHelper> m_cubemap_helper;

    void setup_cubemap_info();

    std::vector<std::shared_ptr<Phos::Texture>> m_cubemap_face_textures; // left, right, top, bottom, front, back
    std::vector<ImTextureID> m_cubemap_face_ids;

    std::shared_ptr<Phos::Texture> m_cubemap_equirectangular_texture;
    ImTextureID m_cubemap_equirectangular_id{};

    // Prefab type
    std::shared_ptr<EditorPrefabHelper> m_prefab_helper;

    // Render functions
    void render_texture_asset() const;
    void render_material_asset() const;
    void render_cubemap_asset();
    void render_prefab_asset() const;
};