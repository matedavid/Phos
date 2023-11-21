#include "asset_importer.h"

#include <fstream>

#include "asset/asset.h"
#include "asset_tools/assimp_importer.h"
#include "asset_tools/asset_builder.h"

#define IS_TEXTURE(ext) (ext == ".jpg" || ext == ".png" || ext == ".jpeg")
#define IS_MODEL(ext) (ext == ".fbx" || ext == ".obj" || ext == ".gltf")
#define IS_SCRIPT(ext) (ext == ".cs")

std::filesystem::path AssetImporter::import_asset(const std::filesystem::path& asset_path,
                                                  const std::filesystem::path& containing_folder) {
    const auto extension = asset_path.extension();
    if (IS_TEXTURE(extension))
        return import_texture(asset_path, containing_folder);
    if (IS_MODEL(extension))
        return import_model(asset_path, containing_folder);
    if (IS_SCRIPT(extension))
        return import_script(asset_path, containing_folder);

    PS_ERROR("[AssetImporter] Unsupported file extension: {}", asset_path.extension().string());
    return {};
}

bool AssetImporter::is_automatic_importable_asset(const std::string& extension) {
    return IS_TEXTURE(extension) || IS_SCRIPT(extension);
}

//
// Specific asset imports
//

std::filesystem::path AssetImporter::import_texture(const std::filesystem::path& asset_path,
                                                    const std::filesystem::path& containing_folder) {
    const auto imported_asset_path = containing_folder / asset_path.filename();
    if (!std::filesystem::exists(imported_asset_path))
        std::filesystem::copy(asset_path, imported_asset_path);

    const auto imported_phos_asset_path = containing_folder / (asset_path.filename().string() + ".psa");

    auto texture_builder = AssetBuilder();

    texture_builder.dump("assetType", "texture");
    texture_builder.dump("id", Phos::UUID());

    const auto relative_path = std::filesystem::relative(imported_asset_path, imported_phos_asset_path.parent_path());
    texture_builder.dump("path", relative_path);

    std::ofstream file(imported_phos_asset_path);
    file << texture_builder;

    return imported_phos_asset_path;
}

std::filesystem::path AssetImporter::import_model(const std::filesystem::path& asset_path,
                                                  const std::filesystem::path& containing_folder) {
    return AssimpImporter::import_model(asset_path, containing_folder);
}

std::filesystem::path AssetImporter::import_script(const std::filesystem::path& asset_path,
                                                   const std::filesystem::path& containing_folder) {
    const auto imported_asset_path = containing_folder / asset_path.filename();
    if (!std::filesystem::exists(imported_asset_path))
        std::filesystem::copy(asset_path, imported_asset_path);

    const auto imported_phos_asset_path = containing_folder / (asset_path.filename().string() + ".psa");

    auto script_builder = AssetBuilder();

    script_builder.dump("assetType", "script");
    script_builder.dump("id", Phos::UUID());

    std::ofstream file(imported_phos_asset_path);
    file << script_builder;

    return imported_phos_asset_path;
}
