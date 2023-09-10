#include "editor_asset_manager.h"

#include <queue>

#include "asset/asset_pack.h"

namespace Phos {

EditorAssetManager::EditorAssetManager(std::string path) : m_path(std::move(path)) {
    m_loader = std::make_unique<AssetLoader>(this);
}

std::shared_ptr<IAsset> EditorAssetManager::load(const std::string& path) {
    const auto id = m_loader->get_id(path);

    if (m_id_to_asset.contains(id)) {
        return m_id_to_asset[id];
    }

    auto asset = m_loader->load(path);
    m_id_to_asset[asset->id] = asset;
    return asset;
}

std::shared_ptr<IAsset> EditorAssetManager::load_by_id(UUID id) {
    if (m_id_to_asset.contains(id)) {
        return m_id_to_asset[id];
    }

    const auto asset = load_by_id_r(id, m_path);
    PS_ASSERT(asset != nullptr, "No asset with id {} found in path: {}", (uint64_t)id, m_path)

    m_id_to_asset[id] = asset;

    return asset;
}

std::shared_ptr<IAsset> EditorAssetManager::load_by_id_r(UUID id, const std::string& folder) const {
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
            return m_loader->load(entry.path());
        }
    }

    while (!pending_directories.empty()) {
        const auto pending_folder = pending_directories.front();
        pending_directories.pop();

        const auto asset = load_by_id_r(id, pending_folder);
        if (asset != nullptr)
            return asset;
    }

    return nullptr;
}

} // namespace Phos
