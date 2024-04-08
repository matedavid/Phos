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

std::vector<std::pair<std::string, std::string>> AssetImporter::get_file_dialog_extensions_filter() {
    const auto join_strings = [](const std::vector<std::string>& values, const std::string& delim) {
        std::string joined;
        for (std::size_t i = 0; i < values.size(); ++i) {
            auto val = values[i];
            val.erase(0, 1); // remove dot
            joined += val;

            if (i != values.size() - 1)
                joined += delim;
        }

        return joined;
    };

    const std::string texture_extensions = join_strings(s_texture_extensions, ",");
    const std::string model_extensions = join_strings(s_model_extensions, ",");
    const std::string script_extensions = join_strings(s_script_extensions, ",");

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

    AssimpImporter::import_model(import_info, containing_folder);

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

    script_builder.dump("assetType", *Phos::AssetType::to_string(Phos::AssetType::Script));
    script_builder.dump("id", Phos::UUID());

    std::ofstream file(imported_phos_asset_path);
    file << script_builder;

    return imported_phos_asset_path;
}
