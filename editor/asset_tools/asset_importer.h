#pragma once

#include <filesystem>

class AssetImporter {
  public:
    static std::filesystem::path import_asset(const std::filesystem::path& asset_path, const std::filesystem::path& containing_folder);

    [[nodiscard]] static bool is_automatic_importable_asset(const std::string& extension);

  private:
    static std::filesystem::path import_texture(const std::filesystem::path& asset_path, const std::filesystem::path& containing_folder);
    static std::filesystem::path import_model(const std::filesystem::path& asset_path, const std::filesystem::path& containing_folder);
    static std::filesystem::path import_script(const std::filesystem::path& asset_path, const std::filesystem::path& containing_folder);
};
