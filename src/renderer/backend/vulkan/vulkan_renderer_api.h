#pragma once

#include <memory>

namespace Phos {

// Forward declarations
class VulkanCommandBuffer;
class VulkanVertexBuffer;
class VulkanIndexBuffer;

class VulkanRendererAPI {
  public:
    static void draw_indexed(const std::shared_ptr<VulkanCommandBuffer>& command_buffer,
                             const std::shared_ptr<VulkanVertexBuffer>& vertex_buffer,
                             const std::shared_ptr<VulkanIndexBuffer>& index_buffer);

    static void draw(const std::shared_ptr<VulkanCommandBuffer>& command_buffer,
                     const std::shared_ptr<VulkanVertexBuffer>& vertex_buffer);
};

} // namespace Phos
