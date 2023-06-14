#pragma once

#include "core.h"

namespace Phos {

// Forward declarations
class StaticMesh;
class CommandBuffer;
class RenderPass;
class Camera;
class Window;
class Framebuffer;

enum class GraphicsAPI {
    Vulkan,
};

struct RendererConfig {
    GraphicsAPI graphics_api = GraphicsAPI::Vulkan; // Vulkan API by default
    std::shared_ptr<Window> window;
};

struct FrameInformation {
    std::shared_ptr<Camera> camera;
    // TODO: std::vector<Light> lights;
};

class INativeRenderer {
  public:
    virtual ~INativeRenderer() = default;

    virtual void begin_frame(const FrameInformation& info) = 0;
    virtual void end_frame() = 0;

    virtual void submit_static_mesh(const std::shared_ptr<CommandBuffer>& command_buffer,
                                    const std::shared_ptr<StaticMesh>& mesh) = 0;

    virtual void begin_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                                   const std::shared_ptr<RenderPass>& render_pass) = 0;

    virtual void end_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                                 const std::shared_ptr<RenderPass>& render_pass) = 0;

    virtual void record_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                                    const std::shared_ptr<RenderPass>& render_pass,
                                    const std::function<void(void)>& func) = 0;

    virtual std::shared_ptr<Framebuffer> presentation_framebuffer() = 0;
};

class Renderer {
  public:
    static void initialize(const RendererConfig& config);
    static void shutdown();

    static void begin_frame(const FrameInformation& info);
    static void end_frame();

    static void submit_static_mesh(const std::shared_ptr<CommandBuffer>& command_buffer,
                                   const std::shared_ptr<StaticMesh>& mesh);

    static void begin_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                                  const std::shared_ptr<RenderPass>& render_pass);

    static void end_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                                const std::shared_ptr<RenderPass>& render_pass);

    static void record_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                                   const std::shared_ptr<RenderPass>& render_pass,
                                   const std::function<void(void)>& func);

    static std::shared_ptr<Framebuffer> presentation_framebuffer();

    static GraphicsAPI graphics_api() { return m_config.graphics_api; }

    static const RendererConfig& config() { return m_config; }

  private:
    static std::shared_ptr<INativeRenderer> m_native_renderer;
    static RendererConfig m_config;
};

} // namespace Phos
