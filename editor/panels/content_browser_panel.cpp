#include "content_browser_panel.h"

#include <algorithm>
#include <ranges>
#include <yaml-cpp/yaml.h>

#include <misc/cpp/imgui_stdlib.h>
#include "imgui/imgui_impl.h"
#include "imgui/imgui_utils.h"

#include "file_dialog.h"
#include "asset_watcher.h"

#include "asset_tools/editor_material_helper.h"
#include "asset_tools/editor_cubemap_helper.h"
#include "asset_tools/editor_prefab_helper.h"
#include "asset_tools/asset_importer.h"

#include "asset/editor_asset_manager.h"

#include "managers/shader_manager.h"

#include "renderer/backend/renderer.h"
#include "renderer/backend/texture.h"

ContentBrowserPanel::ContentBrowserPanel(std::string name,
                                         std::shared_ptr<Phos::EditorAssetManager> asset_manager,
                                         std::shared_ptr<AssetWatcher> asset_watcher)
      : m_name(std::move(name)), m_asset_manager(std::move(asset_manager)), m_asset_watcher(std::move(asset_watcher)) {
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
    const auto components = get_path_components(m_current_path);

    auto partial_path = std::filesystem::path(m_asset_manager->path());
    for (std::size_t i = 0; i < components.size(); ++i) {
        if (i != 0)
            partial_path /= components[i];

        if (i == components.size() - 1) {
            ImGui::Text("%s", components[i].c_str());
        } else {
            if (ImGui::Button(components[i].c_str())) {
                m_current_path = partial_path;
                update();
            }

            const auto payload_asset = ImGuiUtils::drag_drop_target<EditorAsset>("CONTENT_BROWSER_ITEM");
            if (payload_asset.has_value()) {
                move_into_folder(*payload_asset, {.is_directory = true, .path = partial_path, .uuid = Phos::UUID(0)});
            }
        }

        if (i != components.size() - 1) {
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
        const auto asset = *m_assets[i];
        const auto name = asset.path.stem();

        const bool current_asset_being_renamed = asset_being_renamed && *m_renaming_asset_idx == i;

        ImGui::TableSetColumnIndex(current_column);
        display_asset(asset, i);

        asset_hovered |= ImGui::IsItemHovered();

        if (asset.is_directory) {
            const auto payload_asset = ImGuiUtils::drag_drop_target<EditorAsset>("CONTENT_BROWSER_ITEM");
            if (payload_asset.has_value()) {
                move_into_folder(*payload_asset, asset);
            }
        }

        if (!asset_being_renamed && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            const auto icon = asset.is_directory ? m_directory_icon : m_file_icon;
            ImGui::Image(icon, {THUMBNAIL_SIZE / 2.0f, THUMBNAIL_SIZE / 2.0f}, {0, 0}, {1, 1});

            ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", m_assets[i].get(), sizeof(EditorAsset));
            ImGui::EndDragDropSource();
        }

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
                if (remove_asset(asset))
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

                m_assets.push_back(std::make_unique<EditorAsset>(EditorAsset{
                    .is_directory = true,
                    .path = new_folder_path,
                    .uuid = Phos::UUID(0),
                }));

                m_renaming_asset_idx = m_assets.size() - 1;
                m_renaming_asset_tmp_name = "NewFolder";
            }

            if (ImGui::MenuItem("Material")) {
                create_material("NewMaterial");
            }

            if (ImGui::MenuItem("Cubemap")) {
                create_cubemap("NewCubemap");
            }

            if (ImGui::MenuItem("Prefab")) {
                PS_ERROR("[ContentBrowserPanel] Prefab creation unimplemented");
            }

            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("Import")) {
            import_asset();
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
        if (ImGui::InputText("##RenamingAssetInput",
                             &m_renaming_asset_tmp_name,
                             ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
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

static bool directory_is_internal(const std::filesystem::path& path) {
    const bool scripting_internal =
        path.filename() == "bin" || path.filename() == "obj" || path.filename() == "Properties";

    return scripting_internal;
}

void ContentBrowserPanel::update() {
    m_partial_select_idx = {};
    m_selected_asset_idx = {};
    STOP_RENAMING();
    m_assets.clear();

    for (const auto& path : std::filesystem::directory_iterator(m_current_path)) {
        // Hidden files / folders
        if (path.path().filename().string().starts_with('.'))
            continue;

        // Directories
        if (path.is_directory()) {
            if (directory_is_internal(path.path()))
                continue;

            m_assets.push_back(std::make_unique<EditorAsset>(EditorAsset{
                .is_directory = true,
                .path = path.path(),
                .uuid = Phos::UUID(0),
            }));

            continue;
        }

        const auto extension = path.path().extension();
        if (extension != ".psa") {
            if (!AssetImporter::is_automatic_importable_asset(extension))
                continue;

            // Check if non .psa files have a corresponding phos asset file
            const auto psa_path = path.path().string() + ".psa";
            if (!std::filesystem::exists(psa_path)) {
                PS_WARNING("Non .psa file does not have corresponding psa file: {}, importing automatically",
                           path.path().string());
                const auto new_path = AssetImporter::import_asset(path, m_current_path);
                m_asset_watcher->asset_created(new_path);

                const auto asset_id = m_asset_manager->get_asset_id(new_path);
                m_assets.push_back(std::make_unique<EditorAsset>(EditorAsset{
                    .type = m_asset_manager->get_asset_type(asset_id),
                    .path = new_path,
                    .uuid = asset_id,
                }));
            }

            continue;
        }

        try {
            const auto id = m_asset_manager->get_asset_id(path);

            m_assets.push_back(std::make_unique<EditorAsset>(EditorAsset{
                .type = m_asset_manager->get_asset_type(id),
                .path = path.path(),
                .uuid = id,
            }));
        } catch (std::exception&) {
            PS_ERROR("File '{}' does not have corresponding .psa file\n", path.path().string());
        }
    }

    std::ranges::sort(m_assets, [](const std::unique_ptr<EditorAsset>& a, const std::unique_ptr<EditorAsset>& b) {
        if (a->is_directory && !b->is_directory)
            return true;
        if (!a->is_directory && b->is_directory)
            return false;

        return a->path.stem() < b->path.stem();
    });
}

std::vector<std::string> ContentBrowserPanel::get_path_components(const std::filesystem::path& path) const {
    std::vector<std::string> components;

    auto current = path;
    while (current != m_asset_manager->path()) {
        components.push_back(current.filename());
        current = current.parent_path();
    }

    components.push_back(current.filename());

    std::ranges::reverse(components);
    return components;
}

bool ContentBrowserPanel::remove_asset(const EditorAsset& asset) {
    if (asset.is_directory) {
        // @TODO: Should revisit, maybe do not let user delete folder if not empty

        for (const auto& entry : std::filesystem::recursive_directory_iterator(asset.path)) {
            if (entry.path().extension() == ".psa" && !entry.is_directory())
                m_asset_watcher->asset_removed(entry.path());
        }

        return std::filesystem::remove_all(asset.path);
    }

    m_asset_watcher->asset_removed(asset.path);

    switch (asset.type) {
    default:
    case Phos::AssetType::Texture: {
        const auto node = YAML::LoadFile(asset.path);

        auto texture_path = node["path"].as<std::string>();
        texture_path = asset.path.parent_path() / texture_path;

        return std::filesystem::remove(asset.path) && std::filesystem::remove(texture_path);
    }

    case Phos::AssetType::Model: {
        PS_ERROR("[ContentBrowserPanel::remove_asset] Unimplemented");
        return false;
    }

    case Phos::AssetType::Script: {
        const auto script_file = asset.path.parent_path() / asset.path.stem();
        return std::filesystem::remove(asset.path) && std ::filesystem::remove(script_file);
    }

    case Phos::AssetType::Cubemap:
    case Phos::AssetType::Material:
    case Phos::AssetType::Mesh:
    case Phos::AssetType::Prefab:
    case Phos::AssetType::Shader:
    case Phos::AssetType::Scene:
        return std::filesystem::remove(asset.path);
    }
}

void ContentBrowserPanel::rename_currently_renaming_asset() {
    if (!m_renaming_asset_idx.has_value())
        return;

    const auto asset = *m_assets[*m_renaming_asset_idx];

    const std::string extension = asset.is_directory ? "" : ".psa";
    const auto new_path = asset.path.parent_path() / (m_renaming_asset_tmp_name + extension);
    if (new_path == asset.path) {
        STOP_RENAMING();
        return;
    }

    if (!asset.is_directory && asset.type == Phos::AssetType::Texture) {
        const auto node = YAML::LoadFile(asset.path);
        const auto texture_path = asset.path.parent_path() / node["path"].as<std::string>();

        const auto new_texture_path = texture_path.parent_path() / m_renaming_asset_tmp_name;
        PS_INFO("{} -> {}", texture_path.string(), new_texture_path.string());

        std::filesystem::rename(texture_path, new_texture_path);
    }

    PS_INFO("[ContentBrowserPanel] Renaming asset from {} to {}", asset.path.string(), new_path.string());
    std::filesystem::rename(asset.path, new_path);

    m_asset_watcher->asset_renamed(asset.path, new_path);

    update();
}

void ContentBrowserPanel::import_asset() {
    // @TODO: Should move to another place, maybe near AssetImporter
    constexpr auto asset_filter = "jpg,jpeg,png;fbx,obj,gltf;cs";

    const auto paths = FileDialog::open_file_dialog_multiple(asset_filter);
    if (paths.empty())
        return;

    for (const auto& path : paths) {
        if (!std::filesystem::exists(path)) {
            PS_ERROR("[ContentBrowserPanel] Importing file '{}' does not exist", path.string());
            continue;
        }

        const auto new_path = AssetImporter::import_asset(path, m_current_path);
        m_asset_watcher->asset_created(new_path);

        update();
    }
}

void ContentBrowserPanel::move_into_folder(const EditorAsset& asset, const EditorAsset& move_into) {
    PS_ASSERT(move_into.is_directory, "move_into must be directory")

    auto new_asset_path = move_into.path / asset.path.filename();
    PS_INFO("{} -> {}", asset.path.string(), new_asset_path.string());

    if (asset.type == Phos::AssetType::Texture) {
        const auto node = YAML::LoadFile(asset.path);
        const auto texture_path = asset.path.parent_path() / node["path"].as<std::string>();

        const auto new_texture_path = move_into.path / texture_path.filename();
        PS_INFO("{} -> {}", texture_path.string(), new_texture_path.string());

        std::filesystem::rename(texture_path, new_texture_path);
    }

    std::filesystem::rename(asset.path, new_asset_path);

    m_asset_watcher->asset_renamed(asset.path, new_asset_path);

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

    m_asset_watcher->asset_created(material_path);

    m_assets.push_back(std::make_unique<EditorAsset>(EditorAsset{
        .type = Phos::AssetType::Material,
        .path = material_path,
        .uuid = m_asset_manager->get_asset_id(material_path),
    }));
    m_renaming_asset_idx = m_assets.size() - 1;
    m_renaming_asset_tmp_name = material_path.stem();
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

    m_asset_watcher->asset_created(cubemap_path);

    m_assets.push_back(std::make_unique<EditorAsset>(EditorAsset{
        .type = Phos::AssetType::Cubemap,
        .path = cubemap_path,
        .uuid = m_asset_manager->get_asset_id(cubemap_path),
    }));
    m_renaming_asset_idx = m_assets.size() - 1;
    m_renaming_asset_tmp_name = cubemap_path.stem();
}

void ContentBrowserPanel::create_prefab(const std::string& name, const Phos::Entity& entity) {
    // Find available path for prefab
    auto prefab_path = m_current_path / (name + ".psa");
    uint32_t i = 1;
    while (std::filesystem::exists(prefab_path)) {
        prefab_path = m_current_path / (name + std::to_string(i) + ".psa");
        ++i;
    }

    const auto helper = EditorPrefabHelper::create(entity);
    helper->save(prefab_path);

    m_asset_watcher->asset_created(prefab_path);

    m_assets.push_back(std::make_unique<EditorAsset>(EditorAsset{
        .type = Phos::AssetType::Prefab,
        .path = prefab_path,
        .uuid = m_asset_manager->get_asset_id(prefab_path),
    }));
    m_renaming_asset_idx = m_assets.size() - 1;
    m_renaming_asset_tmp_name = prefab_path.stem();
}
