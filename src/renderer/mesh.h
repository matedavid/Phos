#pragma once

#include "core.h"

#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <assimp/scene.h>

#include "renderer/backend/vulkan/vulkan_buffers.h"

namespace Phos {

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

    [[nodiscard]] const std::unique_ptr<VulkanVertexBuffer<Vertex>>& get_vertex_buffer() const {
        return m_vertex_buffer;
    }
    [[nodiscard]] const std::unique_ptr<VulkanIndexBuffer>& get_index_buffer() const { return m_index_buffer; }

  private:
    std::unique_ptr<VulkanVertexBuffer<Vertex>> m_vertex_buffer;
    std::unique_ptr<VulkanIndexBuffer> m_index_buffer;

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;

    void setup_mesh();
};

} // namespace Phos