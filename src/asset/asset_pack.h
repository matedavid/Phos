#pragma once

#include "core.h"

#include <filesystem>

#include "core/uuid.h"

namespace Phos {

class AssetPack {
  public:
    explicit AssetPack(const std::string& path);
    ~AssetPack() = default;

    [[nodiscard]] std::string path_from_id(UUID id) const;

  private:
    std::filesystem::path m_containing_folder;
    std::unordered_map<UUID, std::string> m_id_to_asset_file;
};

} // namespace Phos
