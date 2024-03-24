#include "editor_asset_manager.h"

#include <yaml-cpp/yaml.h>
#include <queue>
#include <ranges>

#include "asset/asset_registry.h"
#include "asset/asset_loader.h"

namespace Phos {

EditorAssetManager::EditorAssetManager(std::filesystem::path path, std::shared_ptr<AssetRegistry> registry)
      : m_path(std::move(path)), m_registry(std::move(registry)) {
    m_loader = std::make_unique<AssetLoader>(this);
}

EditorAssetManager::~EditorAssetManager() = default;

std::shared_ptr<IAsset> EditorAssetManager::load(const std::filesystem::path& path) {
    const auto full_path = m_path / path;
    const auto id = m_loader->get_id(full_path);
    if (id == UUID(0))
        return nullptr;

    return load_by_id(id);
}

std::shared_ptr<IAsset> EditorAssetManager::load_by_id(UUID id) {
    if (m_id_to_asset.contains(id)) {
        return m_id_to_asset[id];
    }

    const auto asset_path = get_asset_path(id);
    if (!asset_path) {
        PHOS_LOG_ERROR("Could not find path of asset with id {}", static_cast<uint64_t>(id), m_path.string());
        return {};
    }

    auto asset = m_loader->load(m_path / *asset_path);
    if (asset == nullptr) {
        PHOS_LOG_ERROR("Could not load asset with id {} and path {}", static_cast<uint64_t>(id), asset_path->string());
        return nullptr;
    }

    m_registry->register_asset(asset, *asset_path);
    m_id_to_asset[asset->id] = asset;

    return asset;
}

std::shared_ptr<IAsset> EditorAssetManager::load_by_id_force_reload(UUID id) {
    const auto asset_path = get_asset_path(id);
    if (!asset_path) {
        PHOS_LOG_ERROR("No asset with id {} found in path: {}", static_cast<uint64_t>(id), m_path.string());
        return nullptr;
    }

    auto asset = m_loader->load(m_path / *asset_path);
    if (asset == nullptr) {
        PHOS_LOG_ERROR("No asset with id {} found in path: {}", static_cast<uint64_t>(id), m_path.string());
        return nullptr;
    }

    m_id_to_asset[id] = asset;

    return asset;
}

void EditorAssetManager::remove_asset_type_from_cache(AssetType type) {
    auto it = m_id_to_asset.begin();
    while (it != m_id_to_asset.end()) {
        if (it->second->asset_type() == type) {
            it = m_id_to_asset.erase(it);
        } else {
            ++it;
        }
    }
}

std::optional<std::filesystem::path> EditorAssetManager::get_asset_path(UUID id) const {
    const auto registry_path = m_registry->get_asset_path(id);
    return registry_path.has_value() ? registry_path : get_path_from_id_r(id, m_path);
}

std::optional<AssetType> EditorAssetManager::get_asset_type(UUID id) const {
    const auto registry_type = m_registry->get_asset_type(id);
    if (registry_type)
        return *registry_type;

    const auto path = get_asset_path(id);
    if (!path) {
        PHOS_LOG_ERROR("Could not find path of asset with id {}", static_cast<uint64_t>(id));
        return {};
    }

    return m_loader->get_type(m_path / *path);
}

std::optional<std::string> EditorAssetManager::get_asset_name(UUID id) const {
    const auto path = get_asset_path(id);
    if (!path) {
        PHOS_LOG_ERROR("Could not find path of asset with id {}", static_cast<uint64_t>(id));
        return {};
    }

    const auto node = YAML::LoadFile(m_path / *path);

    const auto asset_type = m_loader->get_type(m_path / *path);
    if (asset_type == AssetType::Material || asset_type == AssetType::Shader)
        return node["name"].as<std::string>();
    else
        return path->stem();
}

std::optional<UUID> EditorAssetManager::get_asset_id(const std::filesystem::path& path) const {
    const auto id = m_loader->get_id(m_path / path);
    if (id == UUID(0))
        return {};

    return id;
}

std::optional<std::filesystem::path> EditorAssetManager::get_path_from_id_r(UUID id,
                                                                            const std::filesystem::path& folder) const {
    std::queue<std::filesystem::path> pending_directories;

    for (const auto& entry : std::filesystem::directory_iterator(folder)) {
        if (entry.is_directory()) {
            pending_directories.push(entry.path());
            continue;
        } else if (entry.path().extension() != ".psa") {
            continue;
        }

        // TODO: What if file has asset extension but incorrect format...?
        if (m_loader->get_id(entry.path()) == id) {
            return std::filesystem::relative(entry.path(), m_path);
        }
    }

    while (!pending_directories.empty()) {
        const auto pending_folder = pending_directories.front();
        pending_directories.pop();

        const auto path = get_path_from_id_r(id, pending_folder);
        if (path)
            return path;
    }

    return {};
}

} // namespace Phos
