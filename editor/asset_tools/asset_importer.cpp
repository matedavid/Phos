#include "asset_importer.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

#include "asset/asset.h"

#define IS_TEXTURE(path) path.extension() == ".jpg" || path.extension() == ".png" || path.extension() == ".jpeg"
#define IS_MODEL(path) path.extension() == ".fbx" || path.extension() == ".obj"
#define IS_SCRIPT(path) path.extension() == ".cs"

void AssetImporter::import_asset(const std::filesystem::path& asset_path,
                                 const std::filesystem::path& containing_folder) {
    if (IS_TEXTURE(asset_path))
        import_texture(asset_path, containing_folder);
    else if (IS_MODEL(asset_path))
        import_model(asset_path, containing_folder);
    else if (IS_SCRIPT(asset_path))
        import_script(asset_path, containing_folder);
    else
        PS_ERROR("[AssetImporter] Unsupported file extension: {}", asset_path.extension().string());
}

//
// Specific asset imports
//

void AssetImporter::import_texture(const std::filesystem::path& asset_path,
                                   const std::filesystem::path& containing_folder) {
    const auto imported_asset_path = containing_folder / asset_path.filename();
    if (!std::filesystem::exists(imported_asset_path))
        std::filesystem::copy(asset_path, imported_asset_path);

    const auto imported_phos_asset_path = containing_folder / (asset_path.filename().string() + ".psa");

    YAML::Emitter out;
    out << YAML::BeginMap;

    out << YAML::Key << "assetType" << YAML::Value << "texture";
    out << YAML::Key << "id" << YAML::Value << (uint64_t)Phos::UUID();

    out << YAML::Key << "path" << YAML::Value
        << std::filesystem::relative(imported_asset_path, imported_phos_asset_path.parent_path());

    out << YAML::EndMap;

    std::ofstream file(imported_phos_asset_path);
    file << out.c_str();
}

void AssetImporter::import_model(const std::filesystem::path& asset_path,
                                 const std::filesystem::path& containing_folder) {
    (void)asset_path;
    (void)containing_folder;
    PS_ERROR("[AssetImporter::import_model] Unimplemented");
}

void AssetImporter::import_script(const std::filesystem::path& asset_path,
                                  const std::filesystem::path& containing_folder) {
    const auto imported_asset_path = containing_folder / asset_path.filename();
    if (!std::filesystem::exists(imported_asset_path))
        std::filesystem::copy(asset_path, imported_asset_path);

    const auto imported_phos_asset_path = containing_folder / (asset_path.filename().string() + ".psa");

    YAML::Emitter out;
    out << YAML::BeginMap;

    out << YAML::Key << "assetType" << YAML::Value << "script";
    out << YAML::Key << "id" << YAML::Value << (uint64_t)Phos::UUID();

    out << YAML::EndMap;

    std::ofstream file(imported_phos_asset_path);
    file << out.c_str();
}
