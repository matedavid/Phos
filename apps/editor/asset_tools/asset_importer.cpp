#include "asset_importer.h"

#include <fstream>
#include <numeric>

#include "utility/logging.h"

#include "asset/asset.h"
#include "asset_tools/assimp_importer.h"
#include "asset_tools/asset_builder.h"

static std::vector<std::string> s_texture_extensions{".jpg", ".png", ".jpeg"};
static std::vector<std::string> s_model_extensions{".fbx", ".obj", ".gltf"};
static std::vector<std::string> s_script_extensions{".cs"};

std::filesystem::path AssetImporter::import_asset(const std::filesystem::path& asset_path,
                                                  const std::filesystem::path& containing_folder) {
    const auto extension = asset_path.extension();
    if (is_texture(extension))
        return import_texture(asset_path, containing_folder);
    if (is_script(extension))
        return import_script(asset_path, containing_folder);

    PHOS_LOG_ERROR("Not automatically importable extension: {}", asset_path.extension().string());
    return {};
}

bool AssetImporter::is_automatically_importable_asset(const std::string& extension) {
    return is_texture(extension) || is_script(extension);
}

#define CONTAINS(vec, val) std::ranges::find(vec, val) != vec.end()

bool AssetImporter::is_texture(const std::string& extension) {
    return CONTAINS(s_texture_extensions, extension);
}

bool AssetImporter::is_model(const std::string& extension) {
    return CONTAINS(s_model_extensions, extension);
}

bool AssetImporter::is_script(const std::string& extension) {
    return CONTAINS(s_script_extensions, extension);
}

#define JOIN_STRINGS(vec, delim)                                                                  \
    std::accumulate(std::next(vec.begin()), vec.end(), vec[0], [](std::string a, std::string b) { \
        a.erase(0, 1);                                                                            \
        b.erase(0, 1);                                                                            \
        return a + delim + b;                                                                     \
    })

std::vector<std::pair<std::string, std::string>> AssetImporter::get_file_dialog_extensions_filter() {
    const std::string texture_extensions = JOIN_STRINGS(s_texture_extensions, ",");
    const std::string model_extensions = JOIN_STRINGS(s_model_extensions, ",");
    const std::string script_extensions = JOIN_STRINGS(s_script_extensions, ",");

    const std::string all_extensions = texture_extensions + "," + model_extensions + "," + script_extensions;

    return {
        {"All", all_extensions},
        {"Texture", texture_extensions},
        {"Model", model_extensions},
        {"Script", script_extensions},
    };
}

//
// Specific asset imports
//

bool AssetImporter::import_model(const ImportModelInfo& import_info, const std::filesystem::path& containing_folder) {
    PHOS_LOG_INFO("Importing model: {}", import_info.path.string());
    PHOS_LOG_INFO("    Static Model: {}", import_info.import_static);
    PHOS_LOG_INFO("    Import Materials: {}", import_info.import_materials);

    // TODO: 

    return true;
}

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
