#pragma once

#include <filesystem>
#include <vector>

class AssetImporter {
  public:
    static std::filesystem::path import_asset(const std::filesystem::path& asset_path, const std::filesystem::path& containing_folder);

    [[nodiscard]] static bool is_automatically_importable_asset(const std::string& extension);

    [[nodiscard]] static bool is_texture(const std::string& extension);
    [[nodiscard]] static bool is_model(const std::string& extension);
    [[nodiscard]] static bool is_script(const std::string& extension);

    [[nodiscard]] static std::vector<std::pair<std::string, std::string>> get_file_dialog_extensions_filter();

  private:
    static std::filesystem::path import_texture(const std::filesystem::path& asset_path, const std::filesystem::path& containing_folder);
    static std::filesystem::path import_model(const std::filesystem::path& asset_path, const std::filesystem::path& containing_folder);
    static std::filesystem::path import_script(const std::filesystem::path& asset_path, const std::filesystem::path& containing_folder);
};
