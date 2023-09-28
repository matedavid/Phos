#include "assets_panel.h"

#include <algorithm>
#include <ranges>

#include "imgui/imgui_impl.h"
#include "asset/editor_asset_manager.h"
#include "renderer/backend/texture.h"

AssetsPanel::AssetsPanel(std::string name, std::shared_ptr<Phos::EditorAssetManager> asset_manager)
      : m_name(std::move(name)), m_asset_manager(std::move(asset_manager)) {
    m_file_texture = Phos::Texture::create("../editor/icons/file_icon.png");
    m_directory_texture = Phos::Texture::create("../editor/icons/directory_icon.png");

    m_file_icon = ImGuiImpl::add_texture(m_file_texture);
    m_directory_icon = ImGuiImpl::add_texture(m_directory_texture);

    m_current_path = m_asset_manager->path();
    update();
}

void AssetsPanel::on_imgui_render() {
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
    constexpr uint32_t padding = 16.0f;
    constexpr uint32_t thumbnail_size = 64.0f;
    constexpr uint32_t cell_size = thumbnail_size + padding;

    const auto panel_width = ImGui::GetContentRegionAvail().x;
    auto column_count = static_cast<int32_t>(panel_width / cell_size);
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

    for (const auto& asset : m_assets) {
        const auto name = asset.path.stem();
        ImGui::TableSetColumnIndex(current_column);

        ImGui::BeginGroup();
        {
            const auto current_cursor_x = ImGui::GetCursorPosX();
            ImGui::SetCursorPosX(current_cursor_x + (cell_size - thumbnail_size) * 0.5f);

            const auto icon = asset.is_directory ? m_directory_icon : m_file_icon;
            ImGui::Image(icon, {thumbnail_size, thumbnail_size}, {0, 0}, {1, 1});

            ImGui::SetCursorPosX(current_cursor_x);

            // Asset text
            const auto text_length = ImGui::CalcTextSize(name.c_str()).x;
            float text_cursor_x = current_cursor_x + (cell_size - text_length) * 0.5f;

            // If text does not fit in cell, cut and add ".." to the end
            std::string label_name = name;
            if (text_length > cell_size) {
                text_cursor_x = current_cursor_x;

                while (ImGui::CalcTextSize((label_name + "..").c_str()).x >= cell_size)
                    label_name.pop_back();

                label_name += "..";
            }

            ImGui::SetCursorPosX(text_cursor_x);

            ImGui::Text("%s", label_name.c_str());

            ImGui::SetCursorPosX(current_cursor_x);
        }
        ImGui::EndGroup();

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            ImGui::Image(m_file_icon, {thumbnail_size / 2.0f, thumbnail_size / 2.0f}, {0, 0}, {1, 1});

            const auto uuid = (uint64_t)asset.uuid;
            ImGui::SetDragDropPayload("ASSET_PANEL_ITEM", &uuid, sizeof(uint64_t));
            ImGui::EndDragDropSource();
        }

        asset_hovered |= ImGui::IsItemHovered();

        if (asset.is_directory && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            // Double-click on directory to enter
            m_current_path /= name;
            change_directory = true;
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

    //    // Left click on blank = deselect asset
    //    if (!asset_hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()) {
    //    }

    ImGui::End();

    if (change_directory)
        update();
}

void AssetsPanel::update() {
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

        const auto asset = m_asset_manager->load(path.path());
        m_assets.push_back({
            .type = asset->asset_type(),
            .path = path.path(),
            .uuid = asset->id,
        });
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

std::vector<std::string> AssetsPanel::get_path_components() const {
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
