#ifndef RENDERER_BUFFERS_H
#define RENDERER_BUFFERS_H

#include <memory>
#include <vector>

#include "utility/logging.h"

namespace Phos {

// Forward declarations
class VulkanVertexBuffer;
class VulkanIndexBuffer;
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

class UniformBuffer {
  public:
    virtual ~UniformBuffer() = default;

    template <typename T>
    static std::shared_ptr<UniformBuffer> create() {
        const uint32_t size = sizeof(T);
        return std::dynamic_pointer_cast<UniformBuffer>(std::make_shared<VulkanUniformBuffer>(size));
    }

    template <typename T>
    void update(const T& data) {
        PHOS_ASSERT(sizeof(data) == size(), "Size of data must be equal to size of creation data");
        set_data(&data);
    }

    virtual void set_data(const void* data) = 0;
    virtual void set_data(const void* data, uint32_t size, uint32_t offset_bytes) = 0;

    [[nodiscard]] virtual uint32_t size() const = 0;
};

} // namespace Phos

#endif

#include "renderer/backend/vulkan/vulkan_buffers.h"
