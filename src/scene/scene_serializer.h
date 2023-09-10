#pragma once

#include "core.h"

// Forward declarations
namespace YAML {
class Emitter;
}

namespace Phos {

// Forward declarations
class Scene;
class Entity;

class SceneSerializer {
  public:
    SceneSerializer() = delete;

    static void serialize(const std::shared_ptr<Scene>& scene, const std::string& path);

  private:
    static void serialize_entity(YAML::Emitter& out, const Entity& entity);
};

} // namespace Phos
