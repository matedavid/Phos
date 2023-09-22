#include "assets_panel.h"

#include <algorithm>
#include <ranges>

#include "imgui/imgui_impl.h"
#include "asset/editor_asset_manager.h"
#include "renderer/backend/texture.h"

AssetsPanel::AssetsPanel(std::string name, const std::shared_ptr<Phos::AssetManagerBase>& asset_manager)
      : m_name(std::move(name)) {
    m_asset_manager = std::dynamic_pointer_cast<Phos::EditorAssetManager>(asset_manager);
    PS_ASSERT(m_asset_manager != nullptr, "AssetManagerBase should be of subtype EditorAssetManager")

    m_file_texture = Phos::Texture::create("../editor/icons/file_icon.png");
    m_directory_texture = Phos::Texture::create("../editor/icons/directory_icon.png");

    m_file_icon = ImGuiImpl::add_texture(m_file_texture);
    m_directory_icon = ImGuiImpl::add_texture(m_directory_texture);

    m_current_path = m_asset_manager->path();
    update();
}

void AssetsPanel::on_imgui_render() {
    ImGui::Begin(m_name.c_str());

    if (m_current_path != m_asset_manager->path()) {
        if (ImGui::Button("<-")) {
            m_current_path = m_current_path.parent_path();
            update();
        }
    }

    constexpr uint32_t padding = 16.0f;
    constexpr uint32_t thumbnail_size = 64.0f;
    constexpr uint32_t cell_size = thumbnail_size + padding;

    const auto panel_width = ImGui::GetContentRegionAvail().x;
    auto column_count = static_cast<int32_t>(panel_width / cell_size);
    if (column_count < 1)
        column_count = 1;

    if (!ImGui::BeginTable("AssetsTable", column_count, ImGuiTableFlags_Borders)) {
        ImGui::End();
        return;
    }

    ImGui::TableNextRow();

    bool change_directory = false;
    int32_t current_column = 0;

    for (const auto& asset : m_assets) {
        const auto name = asset.path.stem();
        ImGui::TableSetColumnIndex(current_column);
        ImGui::NextColumn();

        ImGui::BeginGroup();
        {
            const auto current_cursor_x = ImGui::GetCursorPosX();
            ImGui::SetCursorPosX(current_cursor_x + (cell_size - thumbnail_size) * 0.5f);

            // Asset icon
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 0, 0, 0));

            const auto icon = asset.is_directory ? m_directory_icon : m_file_icon;
            ImGui::Image(icon, {thumbnail_size, thumbnail_size}, {0, 0}, {1, 1});

            ImGui::SetCursorPosX(current_cursor_x);

            // Asset text
            const auto text_length = ImGui::CalcTextSize(name.c_str()).x;
            float text_cursor_x = current_cursor_x + (cell_size - text_length) * 0.5f;

            std::string label_name = name;
            if (text_length > cell_size) {
                text_cursor_x = current_cursor_x;

                while (ImGui::CalcTextSize((label_name + "..").c_str()).x >= cell_size) {
                    label_name.pop_back();
                }

                label_name += "..";
            }

            ImGui::SetCursorPosX(text_cursor_x);
            ImGui::Text("%s", label_name.c_str());

            ImGui::SetCursorPosX(current_cursor_x);
            ImGui::PopStyleColor();
        }
        ImGui::EndGroup();

        // Select asset
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {}

        // Double-click on directory to enter
        if (asset.is_directory && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
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
            });

            continue;
        }

        if (path.path().extension() != ".psa")
            continue;

        const auto asset = m_asset_manager->load(path.path());
        m_assets.push_back({
            .type = asset->asset_type(),
            .path = path.path(),
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