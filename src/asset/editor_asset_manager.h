#pragma once

#include <filesystem>
#include <optional>

#include "utility/logging.h"
#include "asset/asset_manager.h"

namespace Phos {

// Forward declarations
class AssetRegistry;
class AssetLoader;

class EditorAssetManager : public AssetManagerBase {
  public:
    explicit EditorAssetManager(std::filesystem::path path, std::shared_ptr<AssetRegistry> registry);
    ~EditorAssetManager() override;

    [[nodiscard]] std::shared_ptr<IAsset> load(const std::filesystem::path& path);
    [[nodiscard]] std::shared_ptr<IAsset> load_by_id(UUID id) override;

    template <typename T>
    [[nodiscard]] std::shared_ptr<T> load_by_id_type_force_reload(UUID id) {
        static_assert(std::is_base_of<IAsset, T>());

        const auto asset = load_by_id_force_reload(id);
        if (asset == nullptr)
            return nullptr;

        const auto type_asset = std::dynamic_pointer_cast<T>(asset);
        PHOS_ASSERT(type_asset != nullptr, "Could not convert asset to type {}", typeid(T).name());

        return type_asset;
    }
    [[nodiscard]] std::shared_ptr<IAsset> load_by_id_force_reload(UUID id);

    void remove_asset_type_from_cache(AssetType type);

    [[nodiscard]] std::optional<std::filesystem::path> get_asset_path(UUID id) const;
    [[nodiscard]] std::optional<AssetType> get_asset_type(UUID id) const;
    [[nodiscard]] std::optional<std::string> get_asset_name(UUID id) const;
    [[nodiscard]] std::optional<UUID> get_asset_id(const std::filesystem::path& path) const;

    [[nodiscard]] std::string path() const { return m_path; }
    [[nodiscard]] std::shared_ptr<AssetRegistry> asset_registry() const { return m_registry; }

  private:
    std::filesystem::path m_path;
    std::shared_ptr<AssetRegistry> m_registry;
    std::unique_ptr<AssetLoader> m_loader;

    std::unordered_map<UUID, std::shared_ptr<IAsset>> m_id_to_asset;

    [[nodiscard]] std::optional<std::filesystem::path> get_path_from_id_r(UUID id,
                                                                          const std::filesystem::path& folder) const;
};

} // namespace Phos
