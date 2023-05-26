#pragma once

#include "core.h"

namespace Phos {

// Forward declarations
class VulkanVertexBuffer;
class VulkanCommandBuffer;

class VertexBuffer {
  public:
    virtual ~VertexBuffer() = default;

    template <typename T>
    static std::shared_ptr<VertexBuffer> create(const std::vector<T>& data) {
        return std::make_shared<VulkanVertexBuffer>(data);
    }

    virtual void bind(const std::shared_ptr<VulkanCommandBuffer>& command_buffer) const = 0;
    [[nodiscard]] virtual uint32_t get_size() const = 0;

    template <typename T>
    [[nodiscard]] std::shared_ptr<T> as() {
        static_assert(std::is_base_of_v<VertexBuffer, T>, "Templated argument must be of type VertexBuffer");

        T* value = dynamic_cast<T*>(this);
        return std::shared_ptr<T>(value);
    }
};

} // namespace Phos
