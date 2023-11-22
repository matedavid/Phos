#pragma once

#include "core.h"

// Forward declarations
namespace Phos {
class Entity;
class EditorAssetManager;
} // namespace Phos

class EntityComponentsRenderer {
  public:
    static void display(Phos::Entity& entity, const std::shared_ptr<Phos::EditorAssetManager>& asset_manager);
};