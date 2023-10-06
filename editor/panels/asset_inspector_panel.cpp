#include "asset_inspector_panel.h"

#include "../imgui/imgui_impl.h"

#include <yaml-cpp/yaml.h>

#include "asset_tools/editor_material_helper.h"

#include "asset/asset.h"
#include "asset/editor_asset_manager.h"

#include "renderer/backend/shader.h"
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
        render_material_asset();
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
    m_material_helper.reset();

    // Action based on asset type
    if (m_selected_asset->is_directory)
        return;

    if (m_selected_asset->type == Phos::AssetType::Texture) {
        m_texture = m_asset_manager->load_by_id_type<Phos::Texture>(m_selected_asset->uuid);
        m_imgui_texture_id = ImGuiImpl::add_texture(m_texture);
    } else if (m_selected_asset->type == Phos::AssetType::Material) {
        m_material_helper = EditorMaterialHelper::open(m_selected_asset->path);
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

void AssetInspectorPanel::render_material_asset() const {
    ImGui::Text("%s (Material)", m_material_helper->get_material_name().c_str());

    ImGui::AlignTextToFramePadding();

    // @TODO: Should depend on material shader
    ImGui::Text("Shader:");
    ImGui::SameLine();

    std::string shader_name = "PBR.Geometry.Deferred";
    ImGui::InputText("##MaterialShaderInput", shader_name.data(), shader_name.length(), ImGuiInputTextFlags_ReadOnly);

    ImGui::Separator();

    if (!ImGui::BeginTable("##MaterialPropertiesTable", 2))
        return;

    ImGui::TableNextRow();

    const auto& properties = m_material_helper->get_properties();
    for (const auto& property : properties) {
        auto display_name = property.name;
        // If name is something like 'struct.property', only display 'property'
        if (property.name.find('.') != std::string::npos) {
            display_name = property.name.substr(property.name.find('.') + 1, property.name.length() - 1);
        }

        ImGui::AlignTextToFramePadding();

        ImGui::TableSetColumnIndex(0);

        const auto id = "##" + display_name;
        ImGui::TextUnformatted(display_name.c_str());

        ImGui::TableSetColumnIndex(1);

        if (property.type == Phos::ShaderProperty::Type::Float) {
            auto& data = m_material_helper->fetch<float>(property.name);
            ImGui::InputFloat(id.c_str(), &data);
        } else if (property.type == Phos::ShaderProperty::Type::Vec3) {
            auto& data = m_material_helper->fetch<glm::vec3>(property.name);
            ImGui::ColorEdit3(id.c_str(), &data[0]);
        } else if (property.type == Phos::ShaderProperty::Type::Vec4) {
            auto& data = m_material_helper->fetch<glm::vec4>(property.name);
            ImGui::ColorEdit4(id.c_str(), &data[0]);
        } else if (property.type == Phos::ShaderProperty::Type::Texture) {
            auto& asset_id = m_material_helper->fetch<Phos::UUID>(property.name);

            std::string asset_name;
            if (asset_id != Phos::UUID(0))
                asset_name = m_asset_manager->get_asset_name(asset_id);

            ImGui::InputText(id.c_str(), asset_name.data(), asset_name.length(), ImGuiInputTextFlags_ReadOnly);

            // Tooltip for texture name
            if (!asset_name.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
                ImGui::BeginTooltip();
                ImGui::Text("%s", asset_name.c_str());
                ImGui::EndTooltip();
            }

            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
                    auto uuid = Phos::UUID(*(uint64_t*)payload->Data);

                    const auto asset_type = m_asset_manager->get_asset_type(uuid);
                    if (asset_type == Phos::AssetType::Texture)
                        asset_id = uuid;
                }

                ImGui::EndDragDropTarget();
            }
        }

        ImGui::TableNextRow();
    }

    ImGui::EndTable();

    // @TODO: Temporal button...??
    if (ImGui::Button("Save")) {
        m_material_helper->save();
    }
}
