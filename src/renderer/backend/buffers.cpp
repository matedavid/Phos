#include "buffers.h"

#include "renderer/backend/vulkan/vulkan_buffers.h"

namespace Phos {

std::shared_ptr<IndexBuffer> IndexBuffer::create(const std::vector<uint32_t>& data) {
    return std::make_shared<VulkanIndexBuffer>(data);
}

} // namespace Phos
