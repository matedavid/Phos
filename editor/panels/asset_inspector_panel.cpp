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
    if (m_selected_asset->uuid == asset->uuid)
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
        m_material_info = parse_material_info(m_selected_asset->path);
    } else {
        PS_FAIL("Not implemented")
    }
}

AssetInspectorPanel::MaterialInfo AssetInspectorPanel::parse_material_info(const std::filesystem::path& path) {
    const auto node = YAML::LoadFile(path);

    PS_ASSERT(
        node["assetType"].as<std::string>() == "material", "Path: {}, is not a material asset file", path.string())

    auto material_info = MaterialInfo{};
    material_info.name = node["name"].as<std::string>();

    // @NOTE: Only supporting builtin shaders
    PS_ASSERT(node["shader"]["type"].as<std::string>() == "builtin", "Only builtin shaders supported")
    material_info.shader_name = node["shader"]["name"].as<std::string>();

    const auto properties_node = node["properties"];
    for (const auto it : properties_node) {
        const auto property_name = it.first.as<std::string>();

        const auto property_type = properties_node[it.first]["type"].as<std::string>();
        const auto data_node = properties_node[it.first]["data"];

        if (property_type == "texture") {
            const auto id = Phos::UUID(data_node.as<uint64_t>());

            const auto data = std::make_pair(id, m_asset_manager->get_asset_name(id));
            material_info.texture_properties[property_name] = data;
        } else if (property_type == "vec3") {
            material_info.vec3_properties[property_name] =
                glm::vec3(data_node["x"].as<float>(), data_node["y"].as<float>(), data_node["z"].as<float>());
        } else if (property_type == "vec4") {
            material_info.vec3_properties[property_name] = glm::vec4(data_node["x"].as<float>(),
                                                                     data_node["y"].as<float>(),
                                                                     data_node["z"].as<float>(),
                                                                     data_node["w"].as<float>());
        } else if (property_type == "float") {
            material_info.float_properties[property_name] = data_node.as<float>();
        } else {
            PS_WARNING("Property type '{}' is not valid", property_type);
        }
    }

    return material_info;
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
    ImGui::Text("%s (Material)", m_material_info.name.c_str());

    ImGui::AlignTextToFramePadding();

    ImGui::Text("Shader:");
    ImGui::SameLine();

    auto shader_name = m_material_info.shader_name + " (Builtin shader)";
    ImGui::InputText("##MaterialShader", shader_name.data(), shader_name.length(), ImGuiInputTextFlags_ReadOnly);

    ImGui::Separator();

    // Properties
    if (!ImGui::BeginTable("MaterialInspectorTable", 2))
        return;

    ImGui::TableNextRow();

    // @TODO:

    ImGui::EndTable();
}
