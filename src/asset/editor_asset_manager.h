#pragma once

#include <filesystem>

#include "asset_manager.h"

namespace Phos {

class EditorAssetManager : public AssetManagerBase {
  public:
    explicit EditorAssetManager(std::string path);
    ~EditorAssetManager() override = default;

    [[nodiscard]] std::shared_ptr<IAsset> load(const std::string& path) override;
    [[nodiscard]] std::shared_ptr<IAsset> load_by_id(UUID id) override;
    [[nodiscard]] std::filesystem::path get_asset_path(UUID id) override;

    template <typename T>
    [[nodiscard]] std::shared_ptr<T> load_by_id_type_force_reload(UUID id) {
        static_assert(std::is_base_of<IAsset, T>());

        const auto asset = load_by_id_force_reload(id);
        if (asset == nullptr)
            return nullptr;

        const auto type_asset = std::dynamic_pointer_cast<T>(asset);
        PS_ASSERT(type_asset != nullptr,
                  "[EditorAssetManager::load_by_id_type_force_reload] Could not convert asset to type {}",
                  typeid(T).name())

        return type_asset;
    }
    [[nodiscard]] std::shared_ptr<IAsset> load_by_id_force_reload(UUID id);

    [[nodiscard]] AssetType get_asset_type(UUID id) const;
    [[nodiscard]] std::string get_asset_name(UUID id) const;
    [[nodiscard]] UUID get_asset_id(const std::filesystem::path& path) const;

    [[nodiscard]] std::string path() const { return m_path; }

  private:
    std::string m_path;
    std::unique_ptr<AssetLoader> m_loader;

    std::unordered_map<UUID, std::shared_ptr<IAsset>> m_id_to_asset;

    // std::shared_ptr<IAsset> load_by_id_r(UUID id, const std::string& folder) const;
    [[nodiscard]] std::filesystem::path get_path_from_id_r(UUID id, const std::string& folder) const;
};

} // namespace Phos
