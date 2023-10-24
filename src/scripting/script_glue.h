#pragma once

#include "core.h"

#include <glm/glm.hpp>

namespace Phos {

// Forward declarations
class Scene;

class ScriptGlue {
  public:
    static void initialize();
    static void shutdown();
    static void set_scene(std::shared_ptr<Scene> scene);

  private:
    static void Get_Position(uint64_t id, glm::vec3* out);
};

} // namespace Phos
