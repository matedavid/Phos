#ifndef RENDERER_BUFFERS_H
#define RENDERER_BUFFERS_H

#include "core.h"

namespace Phos {

// Forward declarations
class VulkanVertexBuffer;
class VulkanIndexBuffer;
template <typename T>
class VulkanUniformBuffer;
class VulkanCommandBuffer;

class VertexBuffer {
  public:
    virtual ~VertexBuffer() = default;

    template <typename T>
    static std::shared_ptr<VertexBuffer> create(const std::vector<T>& data) {
        return std::dynamic_pointer_cast<VertexBuffer>(std::make_shared<VulkanVertexBuffer>(data));
    }

    virtual void bind(const std::shared_ptr<VulkanCommandBuffer>& command_buffer) const = 0;
    [[nodiscard]] virtual uint32_t size() const = 0;
};

class IndexBuffer {
  public:
    virtual ~IndexBuffer() = default;

    static std::shared_ptr<IndexBuffer> create(const std::vector<uint32_t>& data);

    virtual void bind(const std::shared_ptr<VulkanCommandBuffer>& command_buffer) const = 0;
    [[nodiscard]] virtual uint32_t count() const = 0;
};

template <typename T>
class UniformBuffer {
  public:
    virtual ~UniformBuffer() = default;

    static std::shared_ptr<UniformBuffer<T>> create() {
        return std::dynamic_pointer_cast<UniformBuffer<T>>(std::make_shared<VulkanUniformBuffer<T>>());
    }

    virtual void update(const T& data) = 0;
    [[nodiscard]] virtual uint32_t size() const = 0;
};

} // namespace Phos

#endif

#include "renderer/backend/vulkan/vulkan_buffers.h"
