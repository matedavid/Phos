#pragma once

#include "core.h"

#include <filesystem>

#include "core/uuid.h"

// Forward declarations
struct aiMaterial;
struct aiScene;
struct aiNode;

namespace YAML {
class Emitter;
}

class AssimpImporter {
  public:
    AssimpImporter() = delete;

    [[nodiscard]] static std::filesystem::path import_model(const std::filesystem::path& path,
                                                            const std::filesystem::path& containing_folder);

  private:
    [[nodiscard]] static Phos::UUID load_mesh(std::size_t idx,
                                              const std::filesystem::path& model_path,
                                              const std::filesystem::path& output_path);

    [[nodiscard]] static Phos::UUID load_material(const aiMaterial* mat,
                                                  std::size_t idx,
                                                  std::unordered_map<std::string, Phos::UUID>& texture_map,
                                                  const std::filesystem::path& parent_model_path,
                                                  const std::filesystem::path& output_path);

    [[nodiscard]] static Phos::UUID load_texture(const std::filesystem::path& texture_path,
                                                 const std::filesystem::path& new_texture_path);

    static void process_model_node_r(YAML::Emitter& out,
                                     const aiNode* node,
                                     const aiScene* scene,
                                     const std::vector<Phos::UUID>& mesh_ids,
                                     const std::vector<Phos::UUID>& material_ids,
                                     std::size_t child_number);
};
