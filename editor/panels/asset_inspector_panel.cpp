#include "asset_inspector_panel.h"

#include "../imgui/imgui_impl.h"

#include <yaml-cpp/yaml.h>

#include "asset/asset.h"
#include "asset/editor_asset_manager.h"

#include "renderer/backend/texture.h"

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

    const std::string lock_text = m_locked ? "Unlock" : "Lock";
    if (ImGui::Button(lock_text.c_str()))
        m_locked = !m_locked;

    switch (m_selected_asset->type) {
    case Phos::AssetType::Texture:
        render_texture_asset();
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
    if ((m_selected_asset.has_value() && m_selected_asset->uuid == asset->uuid) || m_locked)
        return;

    m_selected_asset = std::move(asset);

    if (!m_selected_asset.has_value())
        return;

    // Clear not used asset helpers
    m_texture.reset();
    m_material_info = {};

    // Action based on asset type
    if (m_selected_asset->is_directory)
        return;

    if (m_selected_asset->type == Phos::AssetType::Texture) {
        m_texture = m_asset_manager->load_by_id_type<Phos::Texture>(m_selected_asset->uuid);
        m_imgui_texture_id = ImGuiImpl::add_texture(m_texture);
    } else if (m_selected_asset->type == Phos::AssetType::Material) {
        // @TODO:
    } else {
        PS_FAIL("Not implemented")
    }
}

//
// Render functions
//

void AssetInspectorPanel::render_texture_asset() const {
    ImGui::AlignTextToFramePadding();

    ImGui::Image(m_imgui_texture_id, {48, 48});
    ImGui::SameLine();

    // Using stem() to remove ".psa" file extension
    const auto file_name = m_selected_asset->path.stem();
    ImGui::Text("Texture (2d)\n%s", file_name.c_str());
}
