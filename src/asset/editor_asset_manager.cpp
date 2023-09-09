#include "editor_asset_manager.h"

#include <utility>

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

    return asset;
}

std::shared_ptr<IAsset> EditorAssetManager::load_by_id_r(UUID id, const std::string& folder) {}

} // namespace Phos
