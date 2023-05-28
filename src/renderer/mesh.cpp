#include "mesh.h"

#include "renderer/backend/buffers.h"

namespace Phos {

Mesh::Mesh(const aiMesh* mesh) {
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
            vertex.texture_coordinates.y = tc.y;
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
}

void Mesh::setup_mesh() {
    m_vertex_buffer = VertexBuffer::create(m_vertices);
    m_index_buffer = IndexBuffer::create(m_indices);
}

} // namespace Phos
