#pragma once

#include "core.h"

namespace Phos {

// Forward declarations
class Scene;
class Texture;

class ISceneRenderer {
  public:
    virtual ~ISceneRenderer() = default;

    virtual void set_scene(std::shared_ptr<Scene> scene) = 0;
    virtual void render() = 0;
    [[nodiscard]] virtual std::shared_ptr<Texture> output_texture() const = 0;
};

} // namespace Phos
