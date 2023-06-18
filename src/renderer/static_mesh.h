#pragma once

#include "core.h"

#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "renderer/sub_mesh.h"

namespace Phos {

class StaticMesh {
  public:
    explicit StaticMesh(const std::string& path, bool flip_uvs = false);
    ~StaticMesh() = default;

    [[nodiscard]] const std::vector<std::unique_ptr<SubMesh>>& get_sub_meshes() const { return m_meshes; }

  private:
    std::vector<std::unique_ptr<SubMesh>> m_meshes;
    std::string m_directory;

    void process_node_r(aiNode* node, const aiScene* scene);
};

} // namespace Phos
