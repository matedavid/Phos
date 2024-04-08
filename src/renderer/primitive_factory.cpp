#include "primitive_factory.h"

#include <vector>

#include "renderer/mesh.h"

namespace Phos {

void PrimitiveFactory::shutdown() {
    m_cube = nullptr;
}

std::shared_ptr<StaticMesh> PrimitiveFactory::m_cube = nullptr;

std::shared_ptr<StaticMesh> PrimitiveFactory::get_cube() {
    if (m_cube == nullptr) {
        // clang-format off
        const std::vector<SubMesh::Vertex> vertices = {
            // Front face
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{-0.5f, 0.5f, -0.5f}, {0.0f, -0.5f, -1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f, -0.5f}, {0.0f, -0.5f, -1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},

            // Back face
            {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},

            // Right face
            {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},

            // Left face
            {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},
            {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},

            // Top face
            {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},

            // Bottom face
            {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},
        };

        const std::vector<uint32_t> indices = {
            // Front face
            1, 0, 2, 2, 3, 1,
            // Back face
            4, 5, 7, 7, 6, 4,
            // Right face
            8, 9, 11, 11, 10, 8,
            // Left face
            12, 13, 15, 15, 14, 12,
            // Top face
            16, 17, 19, 19, 18, 16,
            // Bottom face
            21, 20, 22, 22, 23, 21,
        };
        // clang-format on

        const auto sub_mesh = std::make_shared<SubMesh>(vertices, indices);
        m_cube = std::make_shared<StaticMesh>(std::vector<std::shared_ptr<SubMesh>>{sub_mesh});
    }

    return m_cube;
}

} // namespace Phos
