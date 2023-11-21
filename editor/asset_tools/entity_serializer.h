#pragma once

#include "core.h"

// Forward declarations
namespace YAML {
class Emitter;
}
namespace Phos {
class Entity;
}

class AssetBuilder;

class EntitySerializer {
  public:
    static AssetBuilder serialize(const Phos::Entity& entity);
};
