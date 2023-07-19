#pragma once

#include "core.h"

#include "core/uuid.h"

#include "asset/asset.h"
#include "asset/asset_loader.h"
#include "asset/asset_pack.h"

namespace Phos {

class AssetManager {
  public:
    explicit AssetManager(std::shared_ptr<AssetPack> asset_pack) : m_asset_pack(std::move(asset_pack)) {
        m_loader = std::make_unique<AssetLoader>(this);
    }

    ~AssetManager() = default;

    std::shared_ptr<IAsset> load(const std::string& path) {
        const auto id = m_loader->get_id(path);

        if (m_id_to_asset.contains(id)) {
            return m_id_to_asset[id];
        }

        auto asset = m_loader->load(path);
        m_id_to_asset[asset->id] = asset;
        return asset;
    }

    template <typename T>
    std::shared_ptr<T> load(const std::string& path) {
        static_assert(std::is_base_of<IAsset, T>());

        const auto asset = std::dynamic_pointer_cast<T>(load(path));
        PS_ASSERT(asset != nullptr, "Could not convert asset to type {}", typeid(T).name())

        return asset;
    }

    template <typename T>
    std::shared_ptr<T> load_by_id(UUID id) {
        static_assert(std::is_base_of<IAsset, T>());

        if (m_id_to_asset.contains(id)) {
            return std::dynamic_pointer_cast<T>(m_id_to_asset[id]);
        }

        const auto path = m_asset_pack->path_from_id(id);
        return load<T>(path);
    }

  private:
    std::unique_ptr<AssetLoader> m_loader;
    std::shared_ptr<AssetPack> m_asset_pack;

    std::unordered_map<UUID, std::shared_ptr<IAsset>> m_id_to_asset;
};

} // namespace Phos
