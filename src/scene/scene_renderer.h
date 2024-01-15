#pragma once

#include <memory>

namespace Phos {

// Forward declarations
class Scene;
class Texture;
class Camera;
class Cubemap;

struct BloomConfig {
    bool enabled = false;
    float threshold = 1.0f;
};

struct EnvironmentConfig {
    std::shared_ptr<Cubemap> skybox = nullptr;
};

struct SceneRendererConfig {
    BloomConfig bloom_config{};
    EnvironmentConfig environment_config{};
};

class ISceneRenderer {
  public:
    virtual ~ISceneRenderer() = default;

    virtual void change_config(const SceneRendererConfig& config) = 0;
    virtual void set_scene(std::shared_ptr<Scene> scene) = 0;

    virtual void render(const std::shared_ptr<Camera>& camera) = 0;
    [[nodiscard]] virtual std::shared_ptr<Texture> output_texture() const = 0;

    virtual void window_resized(uint32_t width, uint32_t height) = 0;
};

} // namespace Phos
