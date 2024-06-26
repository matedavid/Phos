#include "renderer.h"

#include "utility/logging.h"
#include "utility/profiling.h"

#include "managers/texture_manager.h"
#include "managers/shader_manager.h"

#include "renderer/backend/vulkan/vulkan_renderer.h"

namespace Phos {

std::shared_ptr<INativeRenderer> Renderer::m_native_renderer = nullptr;
RendererConfig Renderer::m_config;

std::unique_ptr<TextureManager> Renderer::m_texture_manager = nullptr;
std::unique_ptr<ShaderManager> Renderer::m_shader_manager = nullptr;

void Renderer::initialize(const RendererConfig& config) {
    m_config = config;

    switch (config.graphics_api) {
    case GraphicsAPI::Vulkan:
        m_native_renderer = std::make_shared<VulkanRenderer>(config);
        break;
    default:
        PHOS_FAIL("Vulkan is the only supported api");
    }

    // Managers
    m_texture_manager = std::make_unique<TextureManager>();
    m_shader_manager = std::make_unique<ShaderManager>();
}

void Renderer::shutdown() {
    // Delete managers
    m_texture_manager.reset();
    m_shader_manager.reset();

    // Delete native renderer
    m_native_renderer.reset();
}

void Renderer::wait_idle() {
    m_native_renderer->wait_idle();
}

void Renderer::begin_frame(const FrameInformation& info) {
    PHOS_PROFILE_ZONE_SCOPED_NAMED("Renderer::begin_frame");
    m_native_renderer->begin_frame(info);
}

void Renderer::end_frame() {
    m_native_renderer->end_frame();
}

void Renderer::submit_static_mesh(const std::shared_ptr<CommandBuffer>& command_buffer,
                                  const std::shared_ptr<StaticMesh>& mesh,
                                  const std::shared_ptr<Material>& material) {
    PHOS_PROFILE_ZONE_SCOPED_NAMED("Renderer::submit_static_mesh");
    m_native_renderer->submit_static_mesh(command_buffer, mesh, material);
}

void Renderer::bind_graphics_pipeline(const std::shared_ptr<CommandBuffer>& command_buffer,
                                      const std::shared_ptr<GraphicsPipeline>& pipeline) {
    PHOS_PROFILE_ZONE_SCOPED_NAMED("Renderer::bind_graphics_pipeline");
    m_native_renderer->bind_graphics_pipeline(command_buffer, pipeline);
}

void Renderer::begin_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                                 const std::shared_ptr<RenderPass>& render_pass) {
    PHOS_PROFILE_ZONE_SCOPED_NAMED("Renderer::begin_render_pass");
    m_native_renderer->begin_render_pass(command_buffer, render_pass);
}

void Renderer::end_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                               const std::shared_ptr<RenderPass>& render_pass) {
    m_native_renderer->end_render_pass(command_buffer, render_pass);
}

void Renderer::submit_command_buffer(const std::shared_ptr<CommandBuffer>& command_buffer) {
    PHOS_PROFILE_ZONE_SCOPED_NAMED("Renderer::submit_command_buffer");
    m_native_renderer->submit_command_buffer(command_buffer);
}

void Renderer::draw_screen_quad(const std::shared_ptr<CommandBuffer>& command_buffer) {
    m_native_renderer->draw_screen_quad(command_buffer);
}

uint32_t Renderer::current_frame() {
    return m_native_renderer->current_frame();
}

} // namespace Phos
