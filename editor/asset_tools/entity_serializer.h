#pragma once

#include "core.h"

#include "asset_tools/asset_builder.h"

// Forward declarations
namespace Phos {
class Entity;
}

class EntitySerializer {
  public:
    static AssetBuilder serialize(const Phos::Entity& entity);
};
