#include "core/entry_point.h"
#include "core/layer.h"
#include "core/application.h"

#include "renderer/backend/vulkan/vulkan_renderer.h"
#include "renderer/backend/vulkan/deferred_renderer.h"

class SandboxLayer : public Phos::Layer {
  public:
    SandboxLayer() = default;
    ~SandboxLayer() override = default;

    void on_update([[maybe_unused]] double ts) override { m_renderer.update(); }

  private:
    Phos::DeferredRenderer m_renderer{};
};

constexpr uint32_t WIDTH = 1280;
constexpr uint32_t HEIGHT = 960;

struct V {
    float h;
};

#define As(T, value) std::dynamic_pointer_cast<T>(value)

Phos::Application* create_application() {
    auto* application = new Phos::Application("Phos Engine", WIDTH, HEIGHT);
    application->push_layer<SandboxLayer>();

    auto u = Phos::UniformBuffer<V>::create();
    u->update(V{.h = 4.0f});

    return application;
}
