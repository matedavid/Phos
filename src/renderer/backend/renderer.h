#pragma once

#include "core.h"

namespace Phos {

// Forward declarations
class StaticMesh;
class CommandBuffer;
class RenderPass;
class GraphicsPipeline;
class Camera;
class Window;
class Framebuffer;
class Light;

enum class GraphicsAPI {
    Vulkan,
};

struct RendererConfig {
    GraphicsAPI graphics_api = GraphicsAPI::Vulkan; // Vulkan API by default
    std::shared_ptr<Window> window;
};

struct FrameInformation {
    std::shared_ptr<Camera> camera;
    std::vector<std::shared_ptr<Light>> lights;
};

class INativeRenderer {
  public:
    virtual ~INativeRenderer() = default;

    virtual void begin_frame(const FrameInformation& info) = 0;
    virtual void end_frame() = 0;
    virtual void wait_idle() = 0;

    virtual void submit_static_mesh(const std::shared_ptr<CommandBuffer>& command_buffer,
                                    const std::shared_ptr<StaticMesh>& mesh) = 0;

    virtual void bind_graphics_pipeline(const std::shared_ptr<CommandBuffer>& command_buffer,
                                        const std::shared_ptr<GraphicsPipeline>& pipeline) = 0;

    virtual void bind_push_constant(const std::shared_ptr<CommandBuffer>& command_buffer,
                                    const std::shared_ptr<GraphicsPipeline>& pipeline,
                                    uint32_t size,
                                    const void* data) = 0;

    virtual void begin_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                                   const std::shared_ptr<RenderPass>& render_pass,
                                   bool presentation_target) = 0;

    virtual void end_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                                 const std::shared_ptr<RenderPass>& render_pass) = 0;

    virtual void submit_command_buffer(const std::shared_ptr<CommandBuffer>& command_buffer) = 0;

    virtual void draw_screen_quad(const std::shared_ptr<CommandBuffer>& command_buffer) = 0;

    virtual std::shared_ptr<Framebuffer> current_frame_framebuffer() = 0;
    virtual std::shared_ptr<Framebuffer> presentation_framebuffer() = 0;
};

class Renderer {
  public:
    static void initialize(const RendererConfig& config);
    static void shutdown();
    static void wait_idle();

    static void begin_frame(const FrameInformation& info);
    static void end_frame();

    static void submit_static_mesh(const std::shared_ptr<CommandBuffer>& command_buffer,
                                   const std::shared_ptr<StaticMesh>& mesh);

    static void bind_graphics_pipeline(const std::shared_ptr<CommandBuffer>& command_buffer,
                                       const std::shared_ptr<GraphicsPipeline>& pipeline);

    template <typename T>
    static void bind_push_constant(const std::shared_ptr<CommandBuffer>& command_buffer,
                                   const std::shared_ptr<GraphicsPipeline>& pipeline,
                                   const T& data) {
        m_native_renderer->bind_push_constant(command_buffer, pipeline, sizeof(T), &data);
    }

    static void begin_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                                  const std::shared_ptr<RenderPass>& render_pass,
                                  bool presentation_target = false);

    static void end_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                                const std::shared_ptr<RenderPass>& render_pass);

    static void submit_command_buffer(const std::shared_ptr<CommandBuffer>& command_buffer);

    static void draw_screen_quad(const std::shared_ptr<CommandBuffer>& command_buffer);

    static std::shared_ptr<Framebuffer> current_frame_framebuffer();
    static std::shared_ptr<Framebuffer> presentation_framebuffer();

    static GraphicsAPI graphics_api() { return m_config.graphics_api; }

    static const RendererConfig& config() { return m_config; }

  private:
    static std::shared_ptr<INativeRenderer> m_native_renderer;
    static RendererConfig m_config;
};

} // namespace Phos
