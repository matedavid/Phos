#pragma once

#include <memory>

namespace Phos {

// Forward declarations
class StaticMesh;

class PrimitiveFactory {
  public:
    PrimitiveFactory() = delete;

    static void shutdown();

    [[nodiscard]] static std::shared_ptr<StaticMesh> get_cube();

  private:
    static std::shared_ptr<StaticMesh> m_cube;
};

} // namespace Phos
