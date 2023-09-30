#include "asset_inspector_panel.h"

#include "asset/asset.h"
#include "asset/editor_asset_manager.h"

#include "renderer/backend/material.h"

AssetInspectorPanel::AssetInspectorPanel(std::string name, std::shared_ptr<Phos::EditorAssetManager> asset_manager)
      : m_name(std::move(name)), m_asset_manager(std::move(asset_manager)) {}

void AssetInspectorPanel::on_imgui_render() {
    ImGui::Begin(m_name.c_str());

    if (!m_selected_asset.has_value()) {
        ImGui::End();
        return;
    }

    if (m_selected_asset->is_directory) {
        // @TODO: Render directory asset inspector
        ImGui::End();
        return;
    }

    switch (m_selected_asset->type) {
    case Phos::AssetType::Texture:
        break;
    case Phos::AssetType::Cubemap:
        break;
    case Phos::AssetType::Shader:
        break;
    case Phos::AssetType::Material:
        break;
    case Phos::AssetType::Mesh:
        break;
    case Phos::AssetType::Model:
        break;
    }

    ImGui::End();
}

void AssetInspectorPanel::set_selected_asset(std::optional<EditorAsset> asset) {
    m_selected_asset = std::move(asset);
}
