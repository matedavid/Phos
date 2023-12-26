#include "buffers.h"

#include "renderer/backend/renderer.h"
#include "renderer/backend/vulkan/vulkan_buffers.h"

namespace Phos {

std::shared_ptr<IndexBuffer> IndexBuffer::create(const std::vector<uint32_t>& data) {
    switch (Renderer::graphics_api()) {
    case GraphicsAPI::Vulkan:
        return std::make_shared<VulkanIndexBuffer>(data);
    default:
        PHOS_FAIL("Vulkan is the only supported api");
    }
}

} // namespace Phos
