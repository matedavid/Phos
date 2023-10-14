#pragma once

#include "core.h"

#include "core/uuid.h"

#include "asset/asset.h"
#include "asset/asset_loader.h"

namespace Phos {

// Forward declarations
class AssetLoader;
class AssetPack;

class AssetManagerBase {
  public:
    virtual ~AssetManagerBase() = default;

    template <typename T>
    [[nodiscard]] std::shared_ptr<T> load_type(const std::string& path) {
        static_assert(std::is_base_of<IAsset, T>());

        const auto asset = std::dynamic_pointer_cast<T>(load(path));
        PS_ASSERT(asset != nullptr, "Could not convert asset to type {}", typeid(T).name())

        return asset;
    }

    template <typename T>
    [[nodiscard]] std::shared_ptr<T> load_by_id_type(UUID id) {
        static_assert(std::is_base_of<IAsset, T>());

        const auto asset = std::dynamic_pointer_cast<T>(load_by_id(id));
        PS_ASSERT(asset != nullptr, "Could not convert asset to type {}", typeid(T).name())

        return asset;
    }

    [[nodiscard]] virtual std::shared_ptr<IAsset> load(const std::string& path) = 0;
    [[nodiscard]] virtual std::shared_ptr<IAsset> load_by_id(UUID id) = 0;

    [[nodiscard]] virtual std::filesystem::path get_asset_path(UUID id) = 0;
};

} // namespace Phos
