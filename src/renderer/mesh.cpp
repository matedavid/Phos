#include "mesh.h"

#include "renderer/backend/buffers.h"

namespace Phos {

//
// StaticMesh
//

SubMesh::SubMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
    m_vertex_buffer = VertexBuffer::create(vertices);
    m_index_buffer = IndexBuffer::create(indices);

    m_aabb = AABB{
        .min = glm::vec3(std::numeric_limits<float>::infinity()),
        .max = glm::vec3(-std::numeric_limits<float>::infinity()),
    };

    for (const auto& vertex : vertices) {
        m_aabb.min = glm::min(m_aabb.min, vertex.position);
        m_aabb.max = glm::max(m_aabb.max, vertex.position);
    }
}

//
// StaticMesh
//

StaticMesh::StaticMesh(std::vector<std::shared_ptr<SubMesh>> sub_meshes) : m_sub_meshes(std::move(sub_meshes)) {
    m_aabb = AABB{
        .min = glm::vec3(std::numeric_limits<float>::infinity()),
        .max = glm::vec3(-std::numeric_limits<float>::infinity()),
    };

    for (const auto& sub_mesh : m_sub_meshes) {
        m_aabb.min = glm::min(m_aabb.min, sub_mesh->bounding_box().min);
        m_aabb.max = glm::max(m_aabb.max, sub_mesh->bounding_box().max);
    }
}

} // namespace Phos