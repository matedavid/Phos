#pragma once

#include "core.h"

#include <filesystem>

class AssetImporter {
  public:
    static void import_asset(const std::filesystem::path& asset_path, const std::filesystem::path& containing_folder);

  private:
    static void import_texture(const std::filesystem::path& asset_path, const std::filesystem::path& containing_folder);
    static void import_model(const std::filesystem::path& asset_path, const std::filesystem::path& containing_folder);
};
