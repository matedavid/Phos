#include "runtime_asset_manager.h"

namespace Phos {

RuntimeAssetManager::RuntimeAssetManager(std::shared_ptr<AssetPack> asset_pack) : m_asset_pack(std::move(asset_pack)) {
    m_loader = std::make_unique<AssetLoader>(this);
}

std::shared_ptr<IAsset> RuntimeAssetManager::load(const std::string& path) {
    const auto id = m_loader->get_id(path);

    if (m_id_to_asset.contains(id)) {
        return m_id_to_asset[id];
    }

    auto asset = m_loader->load(path);
    m_id_to_asset[asset->id] = asset;
    return asset;
}

std::shared_ptr<IAsset> RuntimeAssetManager::load_by_id(UUID id) {
    if (m_id_to_asset.contains(id)) {
        return m_id_to_asset[id];
    }

    const auto path = m_asset_pack->path_from_id(id);
    return load(path);
}

} // namespace Phos