#pragma once

#include "core.h"

#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <assimp/scene.h>

namespace Phos {

// Forward declarations
class VertexBuffer;
class IndexBuffer;

class Mesh {
  public:
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texture_coordinates;
        glm::vec3 tangent;
    };

    explicit Mesh(const aiMesh* mesh);
    ~Mesh() = default;

    [[nodiscard]] const std::shared_ptr<VertexBuffer>& get_vertex_buffer() const { return m_vertex_buffer; }
    [[nodiscard]] const std::shared_ptr<IndexBuffer>& get_index_buffer() const { return m_index_buffer; }

  private:
    std::shared_ptr<VertexBuffer> m_vertex_buffer;
    std::shared_ptr<IndexBuffer> m_index_buffer;

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;

    void setup_mesh();
};

} // namespace Phos