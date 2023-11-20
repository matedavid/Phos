#pragma once

#include "core.h"

// Forward declarations
namespace YAML {
class Emitter;
}
namespace Phos {
class Entity;
}

class EntitySerializer {
  public:
    static void serialize(YAML::Emitter& out, const Phos::Entity& entity);
};
