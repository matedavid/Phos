#include "asset_inspector_panel.h"

#include "imgui/imgui_impl.h"
#include "imgui/imgui_utils.h"

#include <yaml-cpp/yaml.h>

#include "asset_tools/editor_material_helper.h"
#include "asset_tools/editor_cubemap_helper.h"
#include "asset_tools/editor_asset.h"

#include "asset/asset.h"
#include "asset/editor_asset_manager.h"

#include "managers/texture_manager.h"

#include "renderer/backend/renderer.h"
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
        render_cubemap_asset();
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
    case Phos::AssetType::Prefab:
        break;
    case Phos::AssetType::Scene:
        break;
    case Phos::AssetType::Script:
        break;
    }

    ImGui::End();
}

void AssetInspectorPanel::set_selected_asset(std::optional<EditorAsset> asset) {
    if ((m_selected_asset.has_value() && m_selected_asset->uuid == asset->uuid) || m_locked)
        return;

    m_selected_asset = std::move(asset);

    // Clear not used asset helpers
    m_texture.reset();
    m_material_helper.reset();
    m_cubemap_helper.reset();
    m_cubemap_face_textures.clear();
    m_cubemap_face_ids.clear();

    if (!m_selected_asset.has_value())
        return;

    // Action based on asset type
    if (m_selected_asset->is_directory)
        return;

    switch (m_selected_asset->type) {
    case Phos::AssetType::Texture:
        m_texture = m_asset_manager->load_by_id_type<Phos::Texture>(m_selected_asset->uuid);
        m_imgui_texture_id = ImGuiImpl::add_texture(m_texture);
        break;
    case Phos::AssetType::Cubemap:
        m_cubemap_helper = EditorCubemapHelper::open(m_selected_asset->path);
        setup_cubemap_info();
        break;
    case Phos::AssetType::Shader:
        break;
    case Phos::AssetType::Material:
        m_material_helper = EditorMaterialHelper::open(m_selected_asset->path);
        break;
    case Phos::AssetType::Mesh:
        break;
    case Phos::AssetType::Model:
        break;
    case Phos::AssetType::Prefab:
        break;
    case Phos::AssetType::Scene:
        break;
    case Phos::AssetType::Script:
        break;
    }
}

#define CUBEMAP_TEX(face)                                                          \
    face == Phos::UUID(0) ? Phos::Renderer::texture_manager()->get_white_texture() \
                          : m_asset_manager->load_by_id_type<Phos::Texture>(face)

void AssetInspectorPanel::setup_cubemap_info() {
    switch (m_cubemap_helper->get_cubemap_type()) {
    case EditorCubemapHelper::Type::Faces: {
        const auto faces = m_cubemap_helper->get_faces();
        m_cubemap_face_textures = {
            CUBEMAP_TEX(faces.left),
            CUBEMAP_TEX(faces.right),
            CUBEMAP_TEX(faces.top),
            CUBEMAP_TEX(faces.bottom),
            CUBEMAP_TEX(faces.front),
            CUBEMAP_TEX(faces.back),
        };

        for (const auto& face_texture : m_cubemap_face_textures) {
            m_cubemap_face_ids.push_back(ImGuiImpl::add_texture(face_texture));
        }
    } break;
    case EditorCubemapHelper::Type::Equirectangular: {
        m_cubemap_equirectangular_texture = CUBEMAP_TEX(m_cubemap_helper->get_equirectangular_id());
        m_cubemap_equirectangular_id = ImGuiImpl::add_texture(m_cubemap_equirectangular_texture);
    } break;
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

            const auto asset = ImGuiUtils::drag_drop_target<EditorAsset>("CONTENT_BROWSER_ITEM");
            if (asset.has_value() && !asset->is_directory && asset->type == Phos::AssetType::Texture) {
                asset_id = asset->uuid;
            }
        }

        ImGui::TableNextRow();
    }

    ImGui::EndTable();

    // @TODO: Temporary button...??
    if (ImGui::Button("Save")) {
        m_material_helper->save();
    }
}

void AssetInspectorPanel::render_cubemap_asset() {
    ImGui::Text("%s (Cubemap)", m_cubemap_helper->get_cubemap_name().c_str());
    ImGui::Separator();

    // Helper functions
    const auto cubemap_face_input = [&](const std::string& name, EditorCubemapHelper::Face face) {
        ImGui::TableNextRow();

        const auto face_id = static_cast<uint32_t>(face);

        ImGui::AlignTextToFramePadding();

        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%s:", name.c_str());

        ImGui::TableSetColumnIndex(1);
        const std::string id = "##CubemapFaceInput" + name;
        std::string asset_name = std::filesystem::path(m_cubemap_face_textures[face_id]->asset_name).stem();

        ImGui::InputText(id.c_str(), asset_name.data(), ImGuiInputTextFlags_ReadOnly);

        // Tooltip for texture name
        if (!asset_name.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::BeginTooltip();
            ImGui::Text("%s", asset_name.c_str());
            ImGui::EndTooltip();
        }

        const auto asset = ImGuiUtils::drag_drop_target<EditorAsset>("CONTENT_BROWSER_ITEM");
        if (asset.has_value() && !asset->is_directory && asset->type == Phos::AssetType::Texture) {
            m_cubemap_helper->update_face(face, asset->uuid);

            const auto texture = m_asset_manager->load_by_id_type<Phos::Texture>(asset->uuid);
            m_cubemap_face_textures[face_id] = texture;
            m_cubemap_face_ids[face_id] = ImGuiImpl::add_texture(texture);
        }

        ImGui::TableSetColumnIndex(2);
        ImGui::Image(m_cubemap_face_ids[face_id], {32, 32});
    };

    const auto cubemap_equirectangular_input = [&]() {
        ImGui::TableNextRow();

        ImGui::AlignTextToFramePadding();

        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Texture:");

        ImGui::TableSetColumnIndex(1);
        const std::string id = "##CubemapEquirectangularInput";
        std::string asset_name = std::filesystem::path(m_cubemap_equirectangular_texture->asset_name).stem();

        ImGui::InputText(id.c_str(), asset_name.data(), ImGuiInputTextFlags_ReadOnly);

        // Tooltip for texture name
        if (!asset_name.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::BeginTooltip();
            ImGui::Text("%s", asset_name.c_str());
            ImGui::EndTooltip();
        }

        const auto asset = ImGuiUtils::drag_drop_target<EditorAsset>("CONTENT_BROWSER_ITEM");
        if (asset.has_value() && !asset->is_directory && asset->type == Phos::AssetType::Texture) {
            m_cubemap_helper->update_equirectangular_id(asset->uuid);

            const auto texture = m_asset_manager->load_by_id_type<Phos::Texture>(asset->uuid);
            m_cubemap_equirectangular_texture = texture;
            m_cubemap_equirectangular_id = ImGuiImpl::add_texture(texture);
        }

        ImGui::TableSetColumnIndex(2);
        ImGui::Image(m_cubemap_equirectangular_id, {32, 32});
    };
    // ================

    constexpr const char* faces_cubemap_name = "Faces";
    constexpr const char* equirectangular_cubemap_name = "Equirectangular";

    std::string selected_type;

    const auto cubemap_type = m_cubemap_helper->get_cubemap_type();
    switch (cubemap_type) {
    default:
    case EditorCubemapHelper::Type::Faces:
        selected_type = faces_cubemap_name;
        break;
    case EditorCubemapHelper::Type::Equirectangular:
        selected_type = equirectangular_cubemap_name;
        break;
    }

    if (ImGui::BeginCombo("##CameraTypeCombo", selected_type.c_str())) {
        if (ImGui::Selectable(faces_cubemap_name, cubemap_type == EditorCubemapHelper::Type::Faces))
            m_cubemap_helper->change_type(EditorCubemapHelper::Type::Faces);

        if (ImGui::Selectable(equirectangular_cubemap_name, cubemap_type == EditorCubemapHelper::Type::Equirectangular))
            m_cubemap_helper->change_type(EditorCubemapHelper::Type::Equirectangular);

        ImGui::EndCombo();
    }

    // Check if type changed
    if (m_cubemap_helper->get_cubemap_type() != cubemap_type) {
        setup_cubemap_info();
    }

    if (!ImGui::BeginTable("##CubemapAssetInspector", 3))
        return;

    if (m_cubemap_helper->get_cubemap_type() == EditorCubemapHelper::Type::Faces) {
        cubemap_face_input("Left", EditorCubemapHelper::Face::Left);
        cubemap_face_input("Right", EditorCubemapHelper::Face::Right);
        cubemap_face_input("Top", EditorCubemapHelper::Face::Top);
        cubemap_face_input("Bottom", EditorCubemapHelper::Face::Bottom);
        cubemap_face_input("Front", EditorCubemapHelper::Face::Front);
        cubemap_face_input("Back", EditorCubemapHelper::Face::Back);
    } else if (m_cubemap_helper->get_cubemap_type() == EditorCubemapHelper::Type::Equirectangular) {
        cubemap_equirectangular_input();
    }

    ImGui::EndTable();

    // @TODO: Temporary button...??
    if (ImGui::Button("Save")) {
        m_cubemap_helper->save();
    }
}
