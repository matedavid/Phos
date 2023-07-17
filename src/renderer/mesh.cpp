#include "mesh.h"

#include "renderer/backend/buffers.h"

namespace Phos {

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
    m_vertex_buffer = VertexBuffer::create(vertices);
    m_index_buffer = IndexBuffer::create(indices);
}

} // namespace Phos