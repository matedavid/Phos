#include "asset_pack.h"

#include <yaml-cpp/yaml.h>

namespace Phos {

AssetPack::AssetPack(std::string path) : m_path(std::move(path)) {
    m_containing_folder = std::filesystem::path(m_path).parent_path();

    const YAML::Node node = YAML::LoadFile(m_path);

    const YAML::Node assets_node = node["assets"];
    for (const auto& asset : assets_node) {
        const auto id = UUID(asset.first.as<uint64_t>());
        const auto asset_path = asset.second.as<std::string>();

        m_id_to_asset_file[id] = asset_path;
    }
}

std::string AssetPack::path_from_id(UUID id) const {
    PS_ASSERT(m_id_to_asset_file.find(id) != m_id_to_asset_file.end(),
              "Asset pack does not contain asset with id {}",
              (uint64_t)id)

    return m_containing_folder / m_id_to_asset_file.find(id)->second;
}

} // namespace Phos
