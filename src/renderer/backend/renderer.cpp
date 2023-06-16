#include "renderer.h"

#include "renderer/backend/vulkan/vulkan_renderer.h"

namespace Phos {

std::shared_ptr<INativeRenderer> Renderer::m_native_renderer = nullptr;
RendererConfig Renderer::m_config;

void Renderer::initialize(const RendererConfig& config) {
    switch (config.graphics_api) {
    case GraphicsAPI::Vulkan:
        m_native_renderer = std::make_shared<VulkanRenderer>(config);
        break;
    default:
        PS_FAIL("Vulkan is the only supported api")
    }

    m_config = config;
}

void Renderer::shutdown() {
    m_native_renderer.reset();
}

void Renderer::begin_frame(const FrameInformation& info) {
    m_native_renderer->begin_frame(info);
}

void Renderer::end_frame() {
    m_native_renderer->end_frame();
}

void Renderer::submit_static_mesh(const std::shared_ptr<CommandBuffer>& command_buffer,
                                  const std::shared_ptr<StaticMesh>& mesh) {
    m_native_renderer->submit_static_mesh(command_buffer, mesh);
}

void Renderer::bind_graphics_pipeline(const std::shared_ptr<CommandBuffer>& command_buffer,
                                      const std::shared_ptr<GraphicsPipeline>& pipeline) {
    m_native_renderer->bind_graphics_pipeline(command_buffer, pipeline);
}

void Renderer::begin_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                                 const std::shared_ptr<RenderPass>& render_pass) {
    m_native_renderer->begin_render_pass(command_buffer, render_pass);
}

void Renderer::end_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                               const std::shared_ptr<RenderPass>& render_pass) {
    m_native_renderer->end_render_pass(command_buffer, render_pass);
}

void Renderer::record_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                                  const std::shared_ptr<RenderPass>& render_pass,
                                  const std::function<void(void)>& func) {
    m_native_renderer->record_render_pass(command_buffer, render_pass, func);
}

void Renderer::submit_command_buffer(const std::shared_ptr<CommandBuffer>& command_buffer) {
    m_native_renderer->submit_command_buffer(command_buffer);
}

void Renderer::draw_screen_quad(const std::shared_ptr<CommandBuffer>& command_buffer) {
    m_native_renderer->draw_screen_quad(command_buffer);
}

std::shared_ptr<Framebuffer> Renderer::current_frame_framebuffer() {
    return m_native_renderer->current_frame_framebuffer();
}

std::shared_ptr<Framebuffer> Renderer::presentation_framebuffer() {
    return m_native_renderer->presentation_framebuffer();
}

} // namespace Phos
