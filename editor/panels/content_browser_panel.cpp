#include "content_browser_panel.h"

#include <algorithm>
#include <ranges>
#include <yaml-cpp/yaml.h>

#include <misc/cpp/imgui_stdlib.h>
#include "imgui/imgui_impl.h"

#include "asset_tools/editor_material_helper.h"
#include "asset_tools/editor_cubemap_helper.h"

#include "asset/editor_asset_manager.h"

#include "managers/shader_manager.h"

#include "renderer/backend/renderer.h"
#include "renderer/backend/texture.h"
#include "renderer/backend/shader.h"

ContentBrowserPanel::ContentBrowserPanel(std::string name, std::shared_ptr<Phos::EditorAssetManager> asset_manager)
      : m_name(std::move(name)), m_asset_manager(std::move(asset_manager)) {
    m_file_texture = Phos::Texture::create("../editor/icons/file_icon.png");
    m_directory_texture = Phos::Texture::create("../editor/icons/directory_icon.png");

    m_file_icon = ImGuiImpl::add_texture(m_file_texture);
    m_directory_icon = ImGuiImpl::add_texture(m_directory_texture);

    m_current_path = m_asset_manager->path();
    update();
}

constexpr uint32_t PADDING = 16.0f;
constexpr uint32_t THUMBNAIL_SIZE = 64.0f;
constexpr uint32_t CELL_SIZE = THUMBNAIL_SIZE + PADDING;

#define STOP_RENAMING()        \
    m_renaming_asset_idx = {}; \
    m_renaming_asset_tmp_name.clear()

void ContentBrowserPanel::on_imgui_render() {
    ImGui::Begin(m_name.c_str());

    // Print current path components
    const auto components = get_path_components();

    for (auto i = (int32_t)components.size() - 1; i >= 0; --i) {
        if (i == 0)
            ImGui::Text("%s", components[i].c_str());
        else {
            if (ImGui::Button(components[i].c_str())) {
                m_current_path = m_asset_manager->path();
                for (auto j = (int32_t)components.size() - 1; j > i; --j) {
                    m_current_path /= components[j];
                }

                update();
            }
        }

        if (i != 0) {
            ImGui::SameLine();
            ImGui::Text(">");
            ImGui::SameLine();
        }
    }

    ImGui::Separator();

    // Print assets
    const auto panel_width = ImGui::GetContentRegionAvail().x;
    auto column_count = static_cast<int32_t>(panel_width / CELL_SIZE);
    if (column_count < 1)
        column_count = 1;

    if (!ImGui::BeginTable("AssetsTable", column_count)) {
        ImGui::End();
        return;
    }

    ImGui::TableNextRow();

    bool change_directory = false;
    int32_t current_column = 0;
    bool asset_hovered = false;

    const bool asset_being_renamed = m_renaming_asset_idx.has_value();

    for (std::size_t i = 0; i < m_assets.size(); ++i) {
        const auto asset = m_assets[i];
        const auto name = asset.path.stem();

        const bool current_asset_being_renamed = asset_being_renamed && *m_renaming_asset_idx == i;

        ImGui::TableSetColumnIndex(current_column);
        display_asset(asset, i);

        if (!asset_being_renamed && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            ImGui::Image(m_file_icon, {THUMBNAIL_SIZE / 2.0f, THUMBNAIL_SIZE / 2.0f}, {0, 0}, {1, 1});

            const auto uuid = (uint64_t)asset.uuid;
            ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", &uuid, sizeof(uint64_t));
            ImGui::EndDragDropSource();
        }

        asset_hovered |= ImGui::IsItemHovered();

        // Double-click on directory to enter
        if (!current_asset_being_renamed && asset.is_directory && ImGui::IsItemHovered() &&
            ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            m_current_path /= name;
            change_directory = true;
        }

        // Single click to select entity
        if (ImGui::IsItemHovered() && m_partial_select_idx == i && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            m_selected_asset_idx = i;
        }

        // Using partially selected variable to emulate the click and release movement
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            m_partial_select_idx = i;
        } else if (ImGui::IsItemHovered() && m_partial_select_idx == i && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            m_partial_select_idx = {};
        }

        // Right click to open asset context menu
        if ((ImGui::IsItemHovered() || m_selected_asset_idx == i) &&
            ImGui::BeginPopupContextItem("##RightClickAsset", ImGuiMouseButton_Right)) {
            // If renaming, apply changes and stop
            if (asset_being_renamed) {
                rename_currently_renaming_asset();
            }

            m_selected_asset_idx = i;

            if (ImGui::MenuItem("Rename")) {
                m_renaming_asset_idx = i;
                m_renaming_asset_tmp_name = name;
            }

            if (ImGui::MenuItem("Delete")) {
                // @TODO: Should ask user for confirmation
                if (std::filesystem::remove(asset.path))
                    update();
                else
                    PS_ERROR("[ContentBrowserPanel] Error removing path {}", asset.path.string());
            }

            ImGui::EndPopup();
        }

        // Hover on icon tooltip with asset name
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay)) {
            ImGui::BeginTooltip();
            ImGui::Text("%s", name.c_str());
            ImGui::EndTooltip();
        }

        ++current_column;
        if (current_column >= column_count) {
            ImGui::TableNextRow();
            current_column = 0;
        }
    }

    ImGui::EndTable();

    // Left click on blank = deselect asset
    if (!asset_hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()) {
        m_partial_select_idx = {};
        m_selected_asset_idx = {};

        // If renaming, apply changes and stop
        if (m_renaming_asset_idx.has_value()) {
            rename_currently_renaming_asset();
        }
    }

    if (!asset_hovered && ImGui::BeginPopupContextWindow("##RightClickPanel", ImGuiPopupFlags_MouseButtonRight)) {
        if (asset_being_renamed) {
            rename_currently_renaming_asset();
        }

        if (ImGui::BeginMenu("Create")) {
            if (ImGui::MenuItem("Folder")) {
                const auto new_folder_path = m_current_path / "NewFolder";
                std::filesystem::create_directory(new_folder_path);

                m_assets.push_back(EditorAsset{
                    .is_directory = true,
                    .path = new_folder_path,
                    .uuid = Phos::UUID(0),
                });

                m_renaming_asset_idx = m_assets.size() - 1;
                m_renaming_asset_tmp_name = "NewFolder";
            }

            if (ImGui::MenuItem("Material")) {
                create_material("NewMaterial");
                update();
            }

            if (ImGui::MenuItem("Cubemap")) {
                create_cubemap("NewCubemap");
                update();
            }

            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }

    ImGui::End();

    if (change_directory)
        update();
}

void ContentBrowserPanel::display_asset(const EditorAsset& asset, std::size_t asset_idx) {
    ImGui::BeginGroup();

    const auto name = asset.path.stem();
    const auto current_cursor_x = ImGui::GetCursorPosX();
    ImGui::SetCursorPosX(current_cursor_x + (CELL_SIZE - THUMBNAIL_SIZE) * 0.5f);

    const auto icon = asset.is_directory ? m_directory_icon : m_file_icon;
    ImGui::Image(icon, {THUMBNAIL_SIZE, THUMBNAIL_SIZE}, {0, 0}, {1, 1});

    ImGui::SetCursorPosX(current_cursor_x);

    if (m_renaming_asset_idx.has_value() && *m_renaming_asset_idx == asset_idx) {
        // Asset renaming input
        ImGui::PushItemWidth(CELL_SIZE + PADDING);
        if (ImGui::InputText(
                "##RenamingAssetInput", &m_renaming_asset_tmp_name, ImGuiInputTextFlags_EnterReturnsTrue)) {
            rename_currently_renaming_asset();
        }
        ImGui::PopItemWidth();
    } else {
        // Asset text
        const auto text_length = ImGui::CalcTextSize(name.c_str()).x;
        float text_cursor_x = current_cursor_x + (CELL_SIZE - text_length) * 0.5f;

        // If text does not fit in cell, cut and add ".." to the end
        std::string label_name = name;
        if (text_length > CELL_SIZE) {
            text_cursor_x = current_cursor_x;

            while (ImGui::CalcTextSize((label_name + "..").c_str()).x >= CELL_SIZE)
                label_name.pop_back();

            label_name += "..";
        }

        ImGui::SetCursorPosX(text_cursor_x);
        ImGui::Text("%s", label_name.c_str());
        ImGui::SetCursorPosX(current_cursor_x);
    }

    ImGui::EndGroup();
}

void ContentBrowserPanel::update() {
    m_partial_select_idx = {};
    m_selected_asset_idx = {};
    STOP_RENAMING();
    m_assets.clear();

    for (const auto& path : std::filesystem::directory_iterator(m_current_path)) {
        if (path.is_directory()) {
            m_assets.push_back({
                .is_directory = true,
                .path = path.path(),
                .uuid = Phos::UUID(0),
            });

            continue;
        }

        if (path.path().extension() != ".psa")
            continue;

        try {
            const auto id = m_asset_manager->get_asset_id(path);

            m_assets.push_back({
                .type = m_asset_manager->get_asset_type(id),
                .path = path.path(),
                .uuid = id,
            });
        } catch (std::exception&) {
            PS_ERROR("Error loading asset with path: {}\n", path.path().string());
        }
    }

    std::ranges::sort(m_assets, [](const EditorAsset& a, const EditorAsset& b) {
        if (a.is_directory && !b.is_directory)
            return true;
        else if (!a.is_directory && b.is_directory)
            return false;
        else
            return a.path.stem() < b.path.stem();
    });
}

std::vector<std::string> ContentBrowserPanel::get_path_components() const {
    std::vector<std::string> components;

    const auto parent_path = m_asset_manager->path();
    auto current = m_current_path;

    while (current != parent_path) {
        components.push_back(current.filename());
        current = current.parent_path();
    }

    components.push_back(current.filename());

    return components;
}

void ContentBrowserPanel::rename_currently_renaming_asset() {
    if (!m_renaming_asset_idx.has_value())
        return;

    const auto asset = m_assets[*m_renaming_asset_idx];

    const std::string extension = asset.is_directory ? "" : ".psa";
    const auto new_path = asset.path.parent_path() / (m_renaming_asset_tmp_name + extension);
    if (new_path == asset.path) {
        STOP_RENAMING();
        return;
    }

    PS_INFO("[ContentBrowserPanel] Renaming asset from {} to {}", asset.path.string(), new_path.string());
    std::filesystem::rename(asset.path, new_path);

    update();
}

void ContentBrowserPanel::create_material(const std::string& name) {
    // @NOTE: Default shader, don't know if the user should be able to select shader beforehand
    const auto shader = Phos::Renderer::shader_manager()->get_builtin_shader("PBR.Geometry.Deferred");

    // Find available path for material
    auto material_path = m_current_path / (name + ".psa");
    uint32_t i = 1;
    while (std::filesystem::exists(material_path)) {
        material_path = m_current_path / (name + std::to_string(i) + ".psa");
        ++i;
    }

    const auto helper = EditorMaterialHelper::create(shader, material_path.stem());
    helper->save(material_path);
}

void ContentBrowserPanel::create_cubemap(const std::string& name) {
    // Find available path for cubemap
    auto cubemap_path = m_current_path / (name + ".psa");
    uint32_t i = 1;
    while (std::filesystem::exists(cubemap_path)) {
        cubemap_path = m_current_path / (name + std::to_string(i) + ".psa");
        ++i;
    }

    const auto helper = EditorCubemapHelper::create(cubemap_path.stem());
    helper->save(cubemap_path);
}
