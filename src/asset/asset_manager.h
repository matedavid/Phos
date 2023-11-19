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

    /*
    template <typename T>
    [[nodiscard]] std::shared_ptr<T> load_type(const std::string& path) {
        static_assert(std::is_base_of<IAsset, T>());

        const auto asset = std::dynamic_pointer_cast<T>(load(path));
        PS_ASSERT(asset != nullptr, "Could not convert asset to type {}", typeid(T).name())

        return asset;
    }
     */

    template <typename T>
    [[nodiscard]] std::shared_ptr<T> load_by_id_type(UUID id) {
        static_assert(std::is_base_of<IAsset, T>());

        const auto asset = load_by_id(id);
        if (asset == nullptr)
            return nullptr;

        const auto type_asset = std::dynamic_pointer_cast<T>(asset);
        PS_ASSERT(type_asset != nullptr,
                  "[AssetManagerBase::load_by_id_type] Could not convert asset to type {}",
                  typeid(T).name())

        return type_asset;
    }

    [[nodiscard]] virtual std::shared_ptr<IAsset> load(const std::string& path) = 0;
    [[nodiscard]] virtual std::shared_ptr<IAsset> load_by_id(UUID id) = 0;

    [[nodiscard]] virtual std::filesystem::path get_asset_path(UUID id) = 0;
};

} // namespace Phos
