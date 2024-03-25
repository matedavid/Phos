#include "asset_registry.h"

#include <fstream>
#include <utility>
#include <yaml-cpp/yaml.h>

#include "utility/logging.h"

namespace Phos {

#define ASSET_REGISTRY_STRICT_UP_TO_DATE

AssetRegistry::AssetRegistry(std::filesystem::path path) : m_path(std::move(path)) {
    reload();
}

std::shared_ptr<AssetRegistry> AssetRegistry::create(std::filesystem::path path) {
    if (std::filesystem::exists(path))
        return std::shared_ptr<AssetRegistry>(new AssetRegistry(std::move(path)));

    std::ofstream file(path);
    if (!file.is_open()) {
        PHOS_LOG_ERROR("Failed to create asset registry file: {}", path.string());
        return nullptr;
    }
    file.close();

    return std::shared_ptr<AssetRegistry>(new AssetRegistry(std::move(path)));
}

void AssetRegistry::register_asset(const std::shared_ptr<IAsset>& asset, std::filesystem::path path) {
    if (m_entries.contains(asset->id)) {
        return;
    }

    m_entries[asset->id] = {
        .id = asset->id,
        .type = asset->asset_type(),
        .path = std::move(path),
    };

#ifdef ASSET_REGISTRY_STRICT_UP_TO_DATE
    dump();
#endif
}

void AssetRegistry::unregister_asset(UUID id) {
    if (!m_entries.contains(id)) {
        return;
    }

    m_entries.erase(id);

#ifdef ASSET_REGISTRY_STRICT_UP_TO_DATE
    dump();
#endif
}

std::optional<std::filesystem::path> AssetRegistry::get_asset_path(UUID id) const {
    if (!m_entries.contains(id)) {
        PHOS_LOG_WARNING("Asset with id {} not found in registry", static_cast<uint64_t>(id));
        return {};
    }

    return m_entries.at(id).path;
}

std::optional<AssetType> AssetRegistry::get_asset_type(UUID id) const {
    if (!m_entries.contains(id)) {
        PHOS_LOG_WARNING("Asset with id {} not found in registry", static_cast<uint64_t>(id));
        return {};
    }

    return m_entries.at(id).type;
}

void AssetRegistry::reload() {
    m_entries.clear();

    const auto node = YAML::LoadFile(m_path);

    for (const auto& entry : node) {
        const auto id = UUID(entry.first.as<uint64_t>());
        const auto type_str = entry.second["type"].as<std::string>();

        const auto asset_type = AssetType::from_string(type_str);
        if (!asset_type) {
            PHOS_LOG_ERROR("Asset type {} is not valid", type_str);
            continue;
        }

        const auto asset_path = entry.second["path"].as<std::string>();

        m_entries[id] = {
            .id = id,
            .type = *asset_type,
            .path = asset_path,
        };
    }
}

void AssetRegistry::dump() const {
    std::vector<RegistryEntry> entries;

    std::ranges::transform(m_entries, std::back_inserter(entries), [](const auto& entry) { return entry.second; });
    std::ranges::sort(entries, [](const auto& lhs, const auto& rhs) {
        return static_cast<uint64_t>(lhs.id) < static_cast<uint64_t>(rhs.id);
    });

    YAML::Node node;
    for (const auto& entry : entries) {
        const auto type_str = AssetType::to_string(entry.type);
        if (!type_str) {
            PHOS_LOG_ERROR("Asset type {} is not valid", static_cast<uint64_t>(entry.type));
            continue;
        }

        YAML::Node yaml_entry;
        yaml_entry["type"] = *type_str;
        yaml_entry["path"] = entry.path.string();

        node[static_cast<uint64_t>(entry.id)] = yaml_entry;
    }

    std::ofstream file(m_path);
    file << node;
}

} // namespace Phos
