#pragma once

#include "core.h"

#include <glm/glm.hpp>

#include "imgui_panel.h"
#include "assets_panel.h"

#include "core/uuid.h"

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

    bool m_locked = false;

    // Texture type
    std::shared_ptr<Phos::Texture> m_texture;
    ImTextureID m_imgui_texture_id{};

    // Material type
    struct MaterialInfo {
        std::string name;
        std::string shader_name;

        std::unordered_map<std::string, float> float_properties;
        std::unordered_map<std::string, glm::vec3> vec3_properties;
        std::unordered_map<std::string, glm::vec4> vec4_properties;
        std::unordered_map<std::string, std::pair<Phos::UUID, std::string>> texture_properties;
    };

    MaterialInfo m_material_info;

    // Render functions
    void render_texture_asset() const;
};
