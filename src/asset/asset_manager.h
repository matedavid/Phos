#pragma once

#include "core.h"

#include "asset/asset.h"
#include "asset/asset_loader.h"

namespace Phos {

class AssetManager {
  public:
    AssetManager() { m_loader = std::make_unique<AssetLoader>(); }
    ~AssetManager() = default;

    std::shared_ptr<IAsset> load(const std::string& path) { return m_loader->load(path); }

    template <typename T>
    std::shared_ptr<T> load(const std::string& path) {
        static_assert(std::is_base_of<IAsset, T>());

        const auto asset = std::dynamic_pointer_cast<T>(m_loader->load(path));
        PS_ASSERT(asset != nullptr, "Could not convert asset to type {}", typeid(T).name());

        return asset;
    }

  private:
    std::unique_ptr<AssetLoader> m_loader;
};

} // namespace Phos
