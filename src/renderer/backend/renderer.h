#pragma once

#include "core.h"

namespace Phos {

// Forward declarations
class StaticMesh;
class CommandBuffer;
class RenderPass;
class Window;

enum class GraphicsAPI {
    Vulkan,
};

struct RendererConfig {
    GraphicsAPI graphics_api;
    std::shared_ptr<Window> window;
};

class INativeRenderer {
  public:
    virtual ~INativeRenderer() = default;

    virtual void submit_static_mesh(const std::shared_ptr<CommandBuffer>& command_buffer,
                                    const std::shared_ptr<StaticMesh>& mesh) = 0;

    virtual void begin_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                                   const std::shared_ptr<RenderPass>& render_pass) = 0;

    virtual void end_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                                 const std::shared_ptr<RenderPass>& render_pass) = 0;

    virtual void record_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                                    const std::shared_ptr<RenderPass>& render_pass,
                                    const std::function<void(void)>& func) = 0;
};

class Renderer {
  public:
    static void initialize(const RendererConfig& config);

    static void shutdown();

    static void submit_static_mesh(const std::shared_ptr<CommandBuffer>& command_buffer,
                                   const std::shared_ptr<StaticMesh>& mesh);

    static void begin_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                                  const std::shared_ptr<RenderPass>& render_pass);

    static void end_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                                const std::shared_ptr<RenderPass>& render_pass);

    static void record_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                                   const std::shared_ptr<RenderPass>& render_pass,
                                   const std::function<void(void)>& func);

    static GraphicsAPI graphics_api() { return m_config.graphics_api; }

  private:
    static std::shared_ptr<INativeRenderer> m_native_renderer;
    static RendererConfig m_config;
};

inline std::shared_ptr<INativeRenderer> Renderer::m_native_renderer = nullptr;
inline RendererConfig Renderer::m_config;

} // namespace Phos
