#include "editor_asset_manager.h"

#include <yaml-cpp/yaml.h>
#include <queue>

#include "asset/asset_pack.h"

namespace Phos {

EditorAssetManager::EditorAssetManager(std::string path) : m_path(std::move(path)) {
    m_loader = std::make_unique<AssetLoader>(this);
}

std::shared_ptr<IAsset> EditorAssetManager::load(const std::string& path) {
    const auto id = m_loader->get_id(path);
    if (id == UUID(0))
        return nullptr;

    return load_by_id(id);
}

std::shared_ptr<IAsset> EditorAssetManager::load_by_id(UUID id) {
    if (m_id_to_asset.contains(id)) {
        return m_id_to_asset[id];
    }

    const auto path = get_path_from_id_r(id, m_path);
    if (!std::filesystem::exists(path)) {
        PS_ERROR("[EditorAssetManager::load_by_id] No asset with id {} found in path: {}", (uint64_t)id, m_path);
        return nullptr;
    }

    auto asset = m_loader->load(path);
    if (asset == nullptr) {
        PS_ERROR("[EditorAssetManager::load_by_id] No asset with id {} found in path: {}", (uint64_t)id, m_path);
        return nullptr;
    }

    m_id_to_asset[asset->id] = asset;
    return asset;
}

std::filesystem::path EditorAssetManager::get_asset_path(UUID id) {
    auto path = get_path_from_id_r(id, m_path);
    if (!std::filesystem::exists(path)) {
        PS_ERROR("[EditorAssetManager::get_asset_path] No asset with id {} found in path: {}", (uint64_t)id, m_path);
        return {};
    }

    return path;
}

std::shared_ptr<IAsset> EditorAssetManager::load_by_id_force_reload(UUID id) {
    const auto path = get_path_from_id_r(id, m_path);
    if (!std::filesystem::exists(path)) {
        PS_ERROR("[EditorAssetManager::load_by_id_force_reload] No asset with id {} found in path: {}",
                 (uint64_t)id,
                 m_path);
        return nullptr;
    }

    auto asset = m_loader->load(path);
    if (asset == nullptr) {
        PS_ERROR("[EditorAssetManager::load_by_id_force_reload] No asset with id {} found in path: {}",
                 (uint64_t)id,
                 m_path);
        return nullptr;
    }

    m_id_to_asset[id] = asset;

    return asset;
}

AssetType EditorAssetManager::get_asset_type(Phos::UUID id) const {
    const auto path = get_path_from_id_r(id, m_path);
    return m_loader->get_type(path);
}

std::string EditorAssetManager::get_asset_name(Phos::UUID id) const {
    const auto path = get_path_from_id_r(id, m_path);
    const auto node = YAML::LoadFile(path);

    const auto asset_type = m_loader->get_type(path);
    if (asset_type == AssetType::Material || asset_type == AssetType::Shader)
        return node["name"].as<std::string>();
    else
        return path.stem();
}

UUID EditorAssetManager::get_asset_id(const std::filesystem::path& path) const {
    return m_loader->get_id(path);
}

std::filesystem::path EditorAssetManager::get_path_from_id_r(UUID id, const std::string& folder) const {
    std::queue<std::string> pending_directories;

    for (const auto& entry : std::filesystem::directory_iterator(folder)) {
        if (entry.is_directory()) {
            pending_directories.push(entry.path());
            continue;
        } else if (entry.path().extension() != ".psa") {
            continue;
        }

        // TODO: What if file has asset extension but incorrect format...?
        if (m_loader->get_id(entry.path()) == id) {
            return entry.path();
        }
    }

    while (!pending_directories.empty()) {
        const auto pending_folder = pending_directories.front();
        pending_directories.pop();

        const auto path = get_path_from_id_r(id, pending_folder);
        if (std::filesystem::exists(path))
            return path;
    }

    return {};
}

} // namespace Phos
