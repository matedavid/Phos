#include "command_buffer.h"

#include "utility/logging.h"

#include "renderer/backend/renderer.h"

#include "renderer/backend/vulkan/vulkan_command_buffer.h"

namespace Phos {

std::shared_ptr<CommandBuffer> CommandBuffer::create() {
    switch (Renderer::graphics_api()) {
    case GraphicsAPI::Vulkan:
        return std::dynamic_pointer_cast<CommandBuffer>(std::make_shared<VulkanCommandBuffer>());
    default:
        PHOS_FAIL("Vulkan is the only supported api");
    }
}

} // namespace Phos
