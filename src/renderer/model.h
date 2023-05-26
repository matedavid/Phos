#pragma once

#include "core.h"

#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "renderer/mesh.h"

namespace Phos {

class Model {
  public:
    explicit Model(const std::string& path, bool flip_uvs = false);
    ~Model() = default;

    [[nodiscard]] const std::vector<std::unique_ptr<Mesh>>& get_meshes() const { return m_meshes; }

  private:
    std::vector<std::unique_ptr<Mesh>> m_meshes;
    std::string m_directory;

    void process_node_r(aiNode* node, const aiScene* scene);
};

} // namespace Phos
