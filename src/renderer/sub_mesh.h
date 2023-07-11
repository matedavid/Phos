#pragma once

#include "core.h"

#include <utility>
#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <assimp/scene.h>

namespace Phos {

// Forward declarations
class VertexBuffer;
class IndexBuffer;
class Material;

class SubMesh {
  public:
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texture_coordinates;
        glm::vec3 tangent;
    };

    explicit SubMesh(const aiMesh* mesh, const aiScene* scene, const std::string& directory);
    ~SubMesh() = default;

    [[nodiscard]] const std::shared_ptr<VertexBuffer>& get_vertex_buffer() const { return m_vertex_buffer; }
    [[nodiscard]] const std::shared_ptr<IndexBuffer>& get_index_buffer() const { return m_index_buffer; }
    [[nodiscard]] const std::shared_ptr<Material>& get_material() const { return m_material; }

    void set_material(std::shared_ptr<Material> material) { m_material = std::move(material); }

  private:
    std::shared_ptr<VertexBuffer> m_vertex_buffer;
    std::shared_ptr<IndexBuffer> m_index_buffer;

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    std::shared_ptr<Material> m_material;

  private:
    void setup_material(const aiMesh* mesh, const aiScene* scene, const std::string& directory);
    void setup_mesh();
};

} // namespace Phos