#pragma once

#include <unordered_map>

#include "asset_manager.h"

namespace Phos {

class RuntimeAssetManager : public AssetManagerBase {
  public:
    explicit RuntimeAssetManager(std::shared_ptr<AssetPack> asset_pack);
    ~RuntimeAssetManager() override = default;

    [[nodiscard]] std::shared_ptr<IAsset> load(const std::string& path) override;
    [[nodiscard]] std::shared_ptr<IAsset> load_by_id(UUID id) override;
    [[nodiscard]] std::filesystem::path get_asset_path(UUID id) override;

  private:
    std::shared_ptr<AssetPack> m_asset_pack;
    std::shared_ptr<AssetLoader> m_loader;

    std::unordered_map<UUID, std::shared_ptr<IAsset>> m_id_to_asset;
};

} // namespace Phos