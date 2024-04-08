#pragma once

#include "asset_manager.h"

namespace Phos {

class RuntimeAssetManager : public AssetManagerBase {
  public:
    explicit RuntimeAssetManager();
    ~RuntimeAssetManager() override = default;

    [[nodiscard]] std::shared_ptr<IAsset> load_by_id(UUID id) override;
};

} // namespace Phos