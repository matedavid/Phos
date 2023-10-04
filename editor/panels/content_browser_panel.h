#pragma once

#include <filesystem>
#include <optional>

#include "imgui_panel.h"

#include "asset/asset.h"

namespace Phos {

// Forward declarations
class EditorAssetManager;
class Texture;

} // namespace Phos

struct EditorAsset {
    bool is_directory = false;
    Phos::AssetType type;
    std::filesystem::path path;
    Phos::UUID uuid;
};

class ContentBrowserPanel : public IImGuiPanel {
  public:
    ContentBrowserPanel(std::string name, std::shared_ptr<Phos::EditorAssetManager> asset_manager);
    ~ContentBrowserPanel() override = default;

    void on_imgui_render() override;

    [[nodiscard]] std::optional<EditorAsset> get_selected_asset() const {
        return m_selected_asset_idx ? m_assets[*m_selected_asset_idx] : std::optional<EditorAsset>();
    }

  private:
    std::string m_name;
    std::shared_ptr<Phos::EditorAssetManager> m_asset_manager;

    std::filesystem::path m_current_path;

    std::shared_ptr<Phos::Texture> m_file_texture;
    std::shared_ptr<Phos::Texture> m_directory_texture;

    ImTextureID m_file_icon{};
    ImTextureID m_directory_icon{};

    std::vector<EditorAsset> m_assets;

    std::optional<std::size_t> m_partial_select_idx;
    std::optional<std::size_t> m_selected_asset_idx;

    void update();
    [[nodiscard]] std::vector<std::string> get_path_components() const;

    void create_material(const std::string& name);
};
