#include "model_loader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "utility/logging.h"

#include "scene/scene.h"

#include "managers/texture_manager.h"
#include "managers/shader_manager.h"

#include "renderer/mesh.h"
#include "renderer/backend/renderer.h"
#include "renderer/backend/material.h"

namespace Phos {

Entity ModelLoader::load_into_scene(const std::string& path, const std::shared_ptr<Scene>& scene, bool flip_uvs) {
    Assimp::Importer importer;

    uint32_t flags =
        aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph;

    if (flip_uvs)
        flags |= aiProcess_FlipUVs;

    const aiScene* loaded_scene = importer.ReadFile(path, flags);

    if (!loaded_scene || loaded_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        PHOS_LOG_ERROR("Error loading model {} with error: {}", path, importer.GetErrorString());
        return {};
    }

    const auto parent_entity = scene->create_entity();

    const auto directory = path.substr(0, path.find_last_of('/')) + "/";
    process_node_r(loaded_scene->mRootNode, loaded_scene, scene, directory, parent_entity);

    return parent_entity;
}

void ModelLoader::process_node_r(const aiNode* node,
                                 const aiScene* loaded_scene,
                                 const std::shared_ptr<Scene>& scene,
                                 const std::string& directory,
                                 Entity parent_entity) {
    for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
        const aiMesh* m = loaded_scene->mMeshes[node->mMeshes[i]];

        Entity child = scene->create_entity();
        child.set_parent(parent_entity);

        const auto mesh = load_mesh(m);
        const auto material = load_material(m, loaded_scene, directory);

        child.add_component<MeshRendererComponent>({.mesh = mesh, .material = material});
    }

    // TODO: Removed temporarily:
    //    for (uint32_t i = 0; i < node->mNumChildren; ++i) {
    //        process_node_r(node->mChildren[i], scene);
    //    }
}

std::shared_ptr<Mesh> ModelLoader::load_mesh(const aiMesh* mesh) {
    std::vector<Mesh::Vertex> vertices;
    std::vector<uint32_t> indices;

    // Vertices
    for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
        Mesh::Vertex vertex{};

        // Position
        const auto& vs = mesh->mVertices[i];
        vertex.position.x = vs.x;
        vertex.position.y = vs.y;
        vertex.position.z = vs.z;

        // Normals
        if (mesh->HasNormals()) {
            const auto& ns = mesh->mNormals[i];
            vertex.normal.x = ns.x;
            vertex.normal.y = ns.y;
            vertex.normal.z = ns.z;
        }

        // Texture coordinates
        if (mesh->HasTextureCoords(0)) {
            const auto& tc = mesh->mTextureCoords[0][i];
            vertex.texture_coordinates.x = tc.x;
            vertex.texture_coordinates.y = 1.0f - tc.y;
        }

        // Tangents
        if (mesh->HasTangentsAndBitangents()) {
            const auto ts = mesh->mTangents[i];
            vertex.tangent.x = ts.x;
            vertex.tangent.y = ts.y;
            vertex.tangent.z = ts.z;
        }

        vertices.push_back(vertex);
    }

    // Indices
    for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
        const aiFace& face = mesh->mFaces[i];

        for (uint32_t index = 0; index < face.mNumIndices; ++index) {
            indices.push_back(face.mIndices[index]);
        }
    }

    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Material> ModelLoader::load_material(const aiMesh* mesh,
                                                     const aiScene* scene,
                                                     const std::string& directory) {
    const aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];

    auto material =
        Material::create(Renderer::shader_manager()->get_builtin_shader("PBR.Geometry.Deferred"), "PBR Deferred");

    // Albedo texture
    aiString albedo_path;
    if (mat->GetTexture(AI_MATKEY_BASE_COLOR_TEXTURE, &albedo_path) == aiReturn_SUCCESS) {
        const std::string path = directory + std::string(albedo_path.C_Str());

        const auto texture = Renderer::texture_manager()->acquire(path);
        material->set("uAlbedoMap", texture);
    }

    // Metallic texture
    aiString metallic_path;
    if (mat->GetTexture(AI_MATKEY_METALLIC_TEXTURE, &metallic_path) == aiReturn_SUCCESS) {
        const std::string path = directory + std::string(metallic_path.C_Str());

        const auto texture = Renderer::texture_manager()->acquire(path);
        material->set("uMetallicMap", texture);
    }

    // Roughness texture
    aiString roughness_path;
    if (mat->GetTexture(AI_MATKEY_ROUGHNESS_TEXTURE, &roughness_path) == aiReturn_SUCCESS) {
        const std::string path = directory + std::string(roughness_path.C_Str());

        const auto texture = Renderer::texture_manager()->acquire(path);
        material->set("uRoughnessMap", texture);
    }

    // AO texture
    aiString ao_path;
    if (mat->GetTexture(aiTextureType_LIGHTMAP, 0, &ao_path) == aiReturn_SUCCESS) {
        const std::string path = directory + std::string(ao_path.C_Str());

        const auto texture = Renderer::texture_manager()->acquire(path);
        material->set("uAOMap", texture);
    }

    // Normal texture
    aiString normal_path;
    if (mat->GetTexture(aiTextureType_NORMALS, 0, &normal_path) == aiReturn_SUCCESS) {
        const std::string path = directory + std::string(normal_path.C_Str());

        const auto texture = Renderer::texture_manager()->acquire(path);
        material->set("uNormalMap", texture);
    }

    [[maybe_unused]] const auto baked = material->bake();
    PHOS_ASSERT(baked, "Failed to build Material for sub_mesh");

    return material;
}

std::shared_ptr<Mesh> ModelLoader::load_single_mesh(const std::string& path) {
    Assimp::Importer importer;

    constexpr uint32_t flags =
        aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph;

    const aiScene* scene = importer.ReadFile(path, flags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        PHOS_LOG_ERROR("Error loading model {} with error: {}", path, importer.GetErrorString());
        return {};
    }

    const auto* mesh = scene->mMeshes[scene->mRootNode->mMeshes[0]];
    return load_mesh(mesh);
}

} // namespace Phos
