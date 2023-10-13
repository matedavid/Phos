#pragma once

#include "core.h"

namespace Phos {

// Forward declarations
class Scene;
class Texture;
class Camera;

struct BloomConfig {
    bool enabled = false;
    float threshold = 1.0f;
};

struct SceneRendererConfig {
    BloomConfig bloom_config{};
};

class ISceneRenderer {
  public:
    virtual ~ISceneRenderer() = default;

    virtual void change_config(SceneRendererConfig config) = 0;
    virtual void set_scene(std::shared_ptr<Scene> scene) = 0;

    virtual void render(const std::shared_ptr<Camera>& camera) = 0;
    [[nodiscard]] virtual std::shared_ptr<Texture> output_texture() const = 0;

    virtual void window_resized(uint32_t width, uint32_t height) = 0;
};

} // namespace Phos
