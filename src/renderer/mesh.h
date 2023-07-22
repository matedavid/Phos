#pragma once

#include "core.h"

#include <glm/glm.hpp>

#include "asset/asset.h"

namespace Phos {

// Forward declarations
class VertexBuffer;
class IndexBuffer;

class Mesh : public IAsset {
  public:
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texture_coordinates;
        glm::vec3 tangent;
    };

    Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
    virtual ~Mesh() = default;

    [[nodiscard]] AssetType asset_type() override { return AssetType::Mesh; }

    [[nodiscard]] const std::shared_ptr<VertexBuffer>& vertex_buffer() const { return m_vertex_buffer; }
    [[nodiscard]] const std::shared_ptr<IndexBuffer>& index_buffer() const { return m_index_buffer; }

  private:
    std::shared_ptr<VertexBuffer> m_vertex_buffer{};
    std::shared_ptr<IndexBuffer> m_index_buffer{};
};

} // namespace Phos
