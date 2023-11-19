#include "runtime_asset_manager.h"

#include "asset/asset_pack.h"

namespace Phos {

RuntimeAssetManager::RuntimeAssetManager(std::shared_ptr<AssetPack> asset_pack) : m_asset_pack(std::move(asset_pack)) {
    m_loader = std::make_unique<AssetLoader>(this);
}

std::shared_ptr<IAsset> RuntimeAssetManager::load(const std::string& path) {
    const auto id = m_loader->get_id(path);
    if (id == UUID(0))
        return nullptr;

    return load_by_id(id);
}

std::shared_ptr<IAsset> RuntimeAssetManager::load_by_id(UUID id) {
    if (m_id_to_asset.contains(id)) {
        return m_id_to_asset[id];
    }

    const auto path = m_asset_pack->path_from_id(id);
    if (!std::filesystem::exists(path)) {
        PS_ERROR("[RuntimeAssetManager::load_by_id] No asset with id {} found", (uint64_t)id);
        return nullptr;
    }

    auto asset = m_loader->load(path);
    if (asset == nullptr) {
        PS_ERROR("[RuntimeAssetManager::load_by_id] No asset with id {} found", (uint64_t)id);
        return nullptr;
    }

    m_id_to_asset[asset->id] = asset;
    return asset;
}

std::filesystem::path RuntimeAssetManager::get_asset_path(UUID id) {
    return m_asset_pack->path_from_id(id);
}

} // namespace Phos