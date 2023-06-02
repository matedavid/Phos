#include "vulkan_renderer.h"

#include "renderer/static_mesh.h"

#include "renderer/backend/vulkan/vulkan_context.h"
#include "renderer/backend/vulkan/vulkan_renderer_api.h"
#include "renderer/backend/vulkan/vulkan_command_buffer.h"
#include "renderer/backend/vulkan/vulkan_buffers.h"
#include "renderer/backend/vulkan/vulkan_render_pass.h"

namespace Phos {

VulkanRenderer::VulkanRenderer(const RendererConfig& config) {
    VulkanContext::init(config.window);
}

VulkanRenderer::~VulkanRenderer() {
    VulkanContext::free();
}

void VulkanRenderer::submit_static_mesh(const std::shared_ptr<CommandBuffer>& command_buffer,
                                        const std::shared_ptr<StaticMesh>& mesh) {
    const auto& native_command_buffer = std::dynamic_pointer_cast<VulkanCommandBuffer>(command_buffer);

    for (const auto& sub : mesh->get_sub_meshes()) {
        const auto& native_vertex_buffer = std::dynamic_pointer_cast<VulkanVertexBuffer>(sub->get_vertex_buffer());
        const auto& native_index_buffer = std::dynamic_pointer_cast<VulkanIndexBuffer>(sub->get_index_buffer());

        VulkanRendererAPI::draw_indexed(native_command_buffer, native_vertex_buffer, native_index_buffer);
    }
}

void VulkanRenderer::begin_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                                       const std::shared_ptr<RenderPass>& render_pass) {
    render_pass->begin(command_buffer);
}

void VulkanRenderer::end_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                                     const std::shared_ptr<RenderPass>& render_pass) {
    render_pass->end(command_buffer);
}

void VulkanRenderer::record_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                                        const std::shared_ptr<RenderPass>& render_pass,
                                        const std::function<void(void)>& func) {
    // Begin Render Pass
    begin_render_pass(command_buffer, render_pass);

    // Call Record Function
    func();

    // End Render Pass
    end_render_pass(command_buffer, render_pass);
}

} // namespace Phos