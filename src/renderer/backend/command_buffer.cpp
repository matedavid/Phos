#include "command_buffer.h"

#include "renderer/backend/vulkan/vulkan_command_buffer.h"

namespace Phos {

std::shared_ptr<CommandBuffer> CommandBuffer::create() {
    return std::dynamic_pointer_cast<CommandBuffer>(std::make_shared<VulkanCommandBuffer>());
}

} // namespace Phos
