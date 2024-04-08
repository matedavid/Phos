#pragma once

#include "core/uuid.h"
#include "utility/logging.h"

#include "asset/asset.h"

namespace Phos {

class AssetManagerBase {
  public:
    virtual ~AssetManagerBase() = default;

    template <typename T>
    [[nodiscard]] std::shared_ptr<T> load_by_id_type(UUID id) {
        static_assert(std::is_base_of<IAsset, T>());

        const auto asset = load_by_id(id);
        if (asset == nullptr)
            return nullptr;

        const auto type_asset = std::dynamic_pointer_cast<T>(asset);
        PHOS_ASSERT(type_asset != nullptr, "Could not convert asset to type {}", typeid(T).name());

        return type_asset;
    }

    [[nodiscard]] virtual std::shared_ptr<IAsset> load_by_id(UUID id) = 0;
};

} // namespace Phos
