#include "mesh.h"

#include "renderer/backend/buffers.h"

namespace Phos {

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
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

} // namespace Phos