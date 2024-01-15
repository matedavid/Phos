#pragma once

#include <filesystem>

// Forward declarations
namespace Phos {
class Scene;
class Entity;
} // namespace Phos

class SceneSerializer {
  public:
    SceneSerializer() = delete;

    static void serialize(const std::shared_ptr<Phos::Scene>& scene, const std::filesystem::path& path);
};
