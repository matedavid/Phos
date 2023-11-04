#pragma once

#include <filesystem>
#include <optional>

#include "imgui_panel.h"

#include "asset/asset.h"
#include "asset_tools/editor_asset.h"

namespace Phos {

// Forward declarations
class EditorAssetManager;
class Texture;

} // namespace Phos

class ContentBrowserPanel : public IImGuiPanel {
  public:
    ContentBrowserPanel(std::string name, std::shared_ptr<Phos::EditorAssetManager> asset_manager);
    ~ContentBrowserPanel() override = default;

    void on_imgui_render() override;

    [[nodiscard]] std::optional<EditorAsset> get_selected_asset() const {
        return m_selected_asset_idx ? *m_assets[*m_selected_asset_idx] : std::optional<EditorAsset>();
    }

  private:
    std::string m_name;
    std::shared_ptr<Phos::EditorAssetManager> m_asset_manager;

    std::filesystem::path m_current_path;

    std::shared_ptr<Phos::Texture> m_file_texture;
    std::shared_ptr<Phos::Texture> m_directory_texture;

    ImTextureID m_file_icon{};
    ImTextureID m_directory_icon{};

    std::vector<std::unique_ptr<EditorAsset>> m_assets;

    std::optional<std::size_t> m_partial_select_idx;
    std::optional<std::size_t> m_selected_asset_idx;

    std::optional<std::size_t> m_renaming_asset_idx;
    std::string m_renaming_asset_tmp_name;

    void display_asset(const EditorAsset& asset, std::size_t asset_idx);

    void update();

    /// Converts a path into a list of its components with respects to m_asset_manager->path().
    /// For example, project/path/to/folder with m_asset_manager->path() == project/ will result in [path, to, folder].
    [[nodiscard]] std::vector<std::string> get_path_components(const std::filesystem::path& path) const;

    bool remove_asset(const EditorAsset& path);
    void rename_currently_renaming_asset();
    void import_asset();
    void move_into_folder(const EditorAsset& asset, const EditorAsset& move_into);

    void create_material(const std::string& name);
    void create_cubemap(const std::string& name);
};
