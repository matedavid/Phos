#pragma once

#include "asset_manager.h"

namespace Phos {

class RuntimeAssetManager : public AssetManagerBase {
  public:
    explicit RuntimeAssetManager(std::shared_ptr<AssetPack> asset_pack);
    ~RuntimeAssetManager() override = default;

    std::shared_ptr<IAssetDescription> load(const std::string& path) override;
    std::shared_ptr<IAssetDescription> load_by_id(UUID id) override;

  private:
    std::shared_ptr<AssetPack> m_asset_pack;
    std::shared_ptr<AssetLoader> m_loader;

    std::unordered_map<UUID, std::shared_ptr<IAssetDescription>> m_id_to_asset;
};

} // namespace Phos