#pragma once

#include <filesystem>
#include "imgui_panel.h"

#include "asset/asset.h"

namespace Phos {

// Forward declarations
class EditorAssetManager;
class Texture;

} // namespace Phos

class AssetsPanel : public IImGuiPanel {
  public:
    AssetsPanel(std::string name, std::shared_ptr<Phos::EditorAssetManager> asset_manager);
    ~AssetsPanel() override = default;

    void on_imgui_render() override;

  private:
    std::string m_name;
    std::shared_ptr<Phos::EditorAssetManager> m_asset_manager;

    std::filesystem::path m_current_path;

    std::shared_ptr<Phos::Texture> m_file_texture;
    std::shared_ptr<Phos::Texture> m_directory_texture;

    ImTextureID m_file_icon{};
    ImTextureID m_directory_icon{};

    struct EditorAsset {
        bool is_directory = false;
        Phos::AssetType type;
        std::filesystem::path path;
        Phos::UUID uuid;
    };

    std::vector<EditorAsset> m_assets;

    void update();
};
