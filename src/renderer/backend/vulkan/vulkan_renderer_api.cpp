#include "vulkan_renderer_api.h"

#include "renderer/backend/vulkan/vulkan_command_buffer.h"
#include "renderer/backend/vulkan/vulkan_buffers.h"

namespace Phos {

void VulkanRendererAPI::draw_indexed(const std::shared_ptr<VulkanCommandBuffer>& command_buffer,
                                     const std::shared_ptr<VulkanVertexBuffer>& vertex_buffer,
                                     const std::shared_ptr<VulkanIndexBuffer>& index_buffer) {
    vertex_buffer->bind(command_buffer);
    index_buffer->bind(command_buffer);

    vkCmdDrawIndexed(command_buffer->handle(), index_buffer->count(), 1, 0, 0, 0);
}

void VulkanRendererAPI::draw(const std::shared_ptr<VulkanCommandBuffer>& command_buffer,
                             const std::shared_ptr<VulkanVertexBuffer>& vertex_buffer) {
    vertex_buffer->bind(command_buffer);
    vkCmdDraw(command_buffer->handle(), vertex_buffer->size(), 1, 0, 0);
}

} // namespace Phos
