#pragma once

#include <filesystem>

#include "core/uuid.h"
#include "asset_tools/asset_importer.h"

// Forward declarations
struct aiMaterial;
struct aiScene;
struct aiNode;
class AssetBuilder;

class AssimpImporter {
  public:
    AssimpImporter() = delete;

    static void import_model(const AssetImporter::ImportModelInfo& import_info,
                             const std::filesystem::path& containing_folder);

  private:
    static void copy_additional_files(const std::filesystem::path& model_path,
                                      const std::filesystem::path& containing_folder);

    [[nodiscard]] static Phos::UUID load_material(const aiMaterial* mat,
                                                  std::unordered_map<std::string, Phos::UUID>& texture_map,
                                                  const std::filesystem::path& parent_model_path,
                                                  const std::filesystem::path& output_path);

    [[nodiscard]] static Phos::UUID load_texture(const std::filesystem::path& texture_path,
                                                 const std::filesystem::path& new_texture_path);

    static AssetBuilder process_model_node_r(const aiNode* node,
                                             const aiScene* scene,
                                             const std::vector<Phos::UUID>& mesh_ids,
                                             const std::vector<Phos::UUID>& material_ids);
};
