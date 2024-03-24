#pragma once

#include <filesystem>
#include <optional>

#include "core/uuid.h"
#include "asset/asset.h"

namespace Phos {

class AssetRegistry {
  public:
    ~AssetRegistry() = default;

    [[nodiscard]] static std::shared_ptr<AssetRegistry> create(std::filesystem::path path);

    void register_asset(const std::shared_ptr<IAsset>& asset, std::filesystem::path path);

    [[nodiscard]] std::optional<std::filesystem::path> get_asset_path(UUID id) const;
    [[nodiscard]] std::optional<AssetType> get_asset_type(UUID id) const;

    void reload();
    void dump() const;

  private:
    std::filesystem::path m_path;

    struct RegistryEntry {
        UUID id;
        AssetType type;
        std::filesystem::path path;
    };
    std::unordered_map<UUID, RegistryEntry> m_entries;

    explicit AssetRegistry(std::filesystem::path path);
};

} // namespace Phos
