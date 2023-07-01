#include "mesh.h"

#include "renderer/sub_mesh.h"

namespace Phos {

Mesh::Mesh(const std::string& path, bool flip_uvs) {
    Assimp::Importer importer;

    uint32_t flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace;
    if (flip_uvs)
        flags |= aiProcess_FlipUVs;

    const aiScene* scene = importer.ReadFile(path, flags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        PS_ERROR("Error loading model {} with error: {}", path, importer.GetErrorString());
        return;
    }

    m_directory = path.substr(0, path.find_last_of('/')) + "/";
    process_node_r(scene->mRootNode, scene);
}

void Mesh::process_node_r(aiNode* node, const aiScene* scene) {
    for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
        const aiMesh* m = scene->mMeshes[node->mMeshes[i]];
        m_meshes.push_back(std::make_unique<SubMesh>(m));
    }

    for (uint32_t i = 0; i < node->mNumChildren; ++i) {
        process_node_r(node->mChildren[i], scene);
    }
}

} // namespace Phos