#include "sub_mesh.h"

#include "managers/texture_manager.h"
#include "managers/shader_manager.h"

#include "renderer/backend/renderer.h"

#include "renderer/backend/buffers.h"
#include "renderer/backend/material.h"
#include "renderer/backend/texture.h"

namespace Phos {

SubMesh::SubMesh(const aiMesh* mesh, const aiScene* scene, const std::string& directory) {
    // Vertices
    for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
        Vertex vertex{};

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

        m_vertices.push_back(vertex);
    }

    // Indices
    for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
        const aiFace& face = mesh->mFaces[i];

        for (uint32_t index = 0; index < face.mNumIndices; ++index) {
            m_indices.push_back(face.mIndices[index]);
        }
    }

    setup_mesh();
    setup_material(mesh, scene, directory);
}

void SubMesh::setup_mesh() {
    m_vertex_buffer = VertexBuffer::create(m_vertices);
    m_index_buffer = IndexBuffer::create(m_indices);
}

void SubMesh::setup_material(const aiMesh* mesh, const aiScene* scene, const std::string& directory) {
    const aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];

    auto definition = Material::Definition{};
    definition.name = "PBR Deferred";
    definition.shader = Renderer::shader_manager()->get_builtin_shader("PBR.Geometry.Deferred");

    // Albedo texture
    aiString albedo_path;
    if (mat->GetTexture(AI_MATKEY_BASE_COLOR_TEXTURE, &albedo_path) == aiReturn_SUCCESS) {
        const std::string path = directory + std::string(albedo_path.C_Str());

        const auto texture = Renderer::texture_manager()->acquire(path);
        definition.textures.insert(std::make_pair("uAlbedoMap", texture));
    }

    // Metallic texture
    aiString metallic_path;
    if (mat->GetTexture(AI_MATKEY_METALLIC_TEXTURE, &metallic_path) == aiReturn_SUCCESS) {
        const std::string path = directory + std::string(metallic_path.C_Str());

        const auto texture = Renderer::texture_manager()->acquire(path);
        definition.textures.insert(std::make_pair("uMetallicMap", texture));
    }

    // Roughness texture
    aiString roughness_path;
    if (mat->GetTexture(AI_MATKEY_ROUGHNESS_TEXTURE, &roughness_path) == aiReturn_SUCCESS) {
        const std::string path = directory + std::string(roughness_path.C_Str());

        const auto texture = Renderer::texture_manager()->acquire(path);
        definition.textures.insert(std::make_pair("uRoughnessMap", texture));
    }

    // AO texture
    aiString ao_path;
    if (mat->GetTexture(aiTextureType_LIGHTMAP, 0, &ao_path) == aiReturn_SUCCESS) {
        const std::string path = directory + std::string(ao_path.C_Str());

        const auto texture = Renderer::texture_manager()->acquire(path);
        definition.textures.insert(std::make_pair("uAOMap", texture));
    }

    // Normal texture
    aiString normal_path;
    if (mat->GetTexture(aiTextureType_NORMALS, 0, &normal_path) == aiReturn_SUCCESS) {
        const std::string path = directory + std::string(normal_path.C_Str());

        const auto texture = Renderer::texture_manager()->acquire(path);
        definition.textures.insert(std::make_pair("uNormalMap", texture));
    }

    m_material = Material::create(definition);
}

} // namespace Phos
