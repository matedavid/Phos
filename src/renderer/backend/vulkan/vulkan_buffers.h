#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <optional>

#include "renderer/backend/buffers.h"

#include "renderer/backend/vulkan/vulkan_device.h"
#include "renderer/backend/vulkan/vulkan_command_buffer.h"
#include "renderer/backend/vulkan/vulkan_buffer.h"

namespace Phos {

// Forward declarations
class VulkanDevice;
class VulkanCommandBuffer;

//
// Vertex Buffer
//
class VulkanVertexBuffer : public VertexBuffer {
  public:
    template <typename T>
    explicit VulkanVertexBuffer(const std::vector<T>& data) {
        const VkDeviceSize size = data.size() * sizeof(T);

        const auto staging_buffer = VulkanBuffer{
            size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        staging_buffer.copy_data(data.data());

        m_buffer = std::make_unique<VulkanBuffer>(size,
                                                  VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        staging_buffer.copy_to_buffer(*m_buffer);

        m_size = (uint32_t)data.size();
    }

    ~VulkanVertexBuffer() override = default;

    void bind(const std::shared_ptr<VulkanCommandBuffer>& command_buffer) const override {
        std::array<VkBuffer, 1> vertex_buffers = {m_buffer->handle()};
        VkDeviceSize offsets[] = {0};

        vkCmdBindVertexBuffers(command_buffer->handle(), 0, vertex_buffers.size(), vertex_buffers.data(), offsets);
    }

    [[nodiscard]] uint32_t size() const override { return m_size; }

    [[nodiscard]] VkBuffer handle() const { return m_buffer->handle(); }

  private:
    std::unique_ptr<VulkanBuffer> m_buffer;
    uint32_t m_size;
};

//
// Index Buffer
//
class VulkanIndexBuffer : public IndexBuffer {
  public:
    explicit VulkanIndexBuffer(const std::vector<uint32_t>& indices);
    ~VulkanIndexBuffer() override = default;

    void bind(const std::shared_ptr<VulkanCommandBuffer>& command_buffer) const override;
    [[nodiscard]] uint32_t count() const override { return m_count; }

    [[nodiscard]] VkBuffer handle() const { return m_buffer->handle(); }

  private:
    std::unique_ptr<VulkanBuffer> m_buffer;
    uint32_t m_count;
};

//
// Uniform Buffer
//

template <typename T>
class VulkanUniformBuffer : public UniformBuffer<T> {
  public:
    VulkanUniformBuffer() {
        m_size = sizeof(T);
        m_buffer =
            std::make_unique<VulkanBuffer>(m_size,
                                           VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        m_buffer->map_memory(m_map_data);
    }

    ~VulkanUniformBuffer() override { m_buffer->unmap_memory(); }

    void update(const T& data) override { memcpy(m_map_data, &data, m_size); }

    [[nodiscard]] uint32_t size() const override { return m_size; }
    [[nodiscard]] VkBuffer handle() const { return m_buffer->handle(); }

  private:
    std::unique_ptr<VulkanBuffer> m_buffer;

    void* m_map_data{nullptr};
    uint32_t m_size;
};

} // namespace Phos
