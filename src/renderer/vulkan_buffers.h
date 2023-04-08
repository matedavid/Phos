#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <optional>

#include "renderer/vulkan_device.h"
#include "renderer/vulkan_command_buffer.h"

// Forward declarations
class VulkanDevice;
class VulkanCommandBuffer;

class BufferUtils {
  public:
    static std::pair<VkBuffer, VkDeviceMemory> create_buffer(const std::shared_ptr<VulkanDevice>& device,
                                                             VkDeviceSize size,
                                                             VkBufferUsageFlags usage,
                                                             VkMemoryPropertyFlags properties);

    static std::optional<uint32_t> find_memory_type(VkPhysicalDevice device,
                                                    uint32_t filter,
                                                    VkMemoryPropertyFlags properties);
};

//
// Vertex Buffer
//
template <typename T>
class VulkanVertexBuffer {
  public:
    VulkanVertexBuffer(std::shared_ptr<VulkanDevice> device, const std::vector<T>& data) : m_device(std::move(device)) {
        const VkDeviceSize size = data.size() * sizeof(T);

        std::tie(m_buffer, m_memory) =
            BufferUtils::create_buffer(m_device,
                                       size,
                                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        m_size = (uint32_t)data.size();

        // TODO: Should look into using a staging buffer

        void* map_data;
        VK_CHECK(vkMapMemory(m_device->handle(), m_memory, 0, size, 0, &map_data))
        memcpy(map_data, data.data(), (size_t)size);
        vkUnmapMemory(m_device->handle(), m_memory);
    }

    ~VulkanVertexBuffer() {
        vkDestroyBuffer(m_device->handle(), m_buffer, nullptr);
        vkFreeMemory(m_device->handle(), m_memory, nullptr);
    }

    void bind(const std::shared_ptr<VulkanCommandBuffer>& command_buffer) const {
        std::array<VkBuffer, 1> vertex_buffers = {m_buffer};
        VkDeviceSize offsets[] = {0};

        vkCmdBindVertexBuffers(command_buffer->handle(), 0, vertex_buffers.size(), vertex_buffers.data(), offsets);
    }

    [[nodiscard]] uint32_t get_size() const { return m_size; }

    [[nodiscard]] VkBuffer handle() const { return m_buffer; }

  private:
    VkBuffer m_buffer{};
    VkDeviceMemory m_memory{};

    uint32_t m_size;

    std::shared_ptr<VulkanDevice> m_device;
};

//
// Index Buffer
//
class VulkanIndexBuffer {
  public:
    VulkanIndexBuffer(std::shared_ptr<VulkanDevice> device, const std::vector<uint32_t>& indices);
    ~VulkanIndexBuffer();

    void bind(const std::shared_ptr<VulkanCommandBuffer>& command_buffer) const;
    [[nodiscard]] uint32_t get_count() const { return m_count; }

    [[nodiscard]] VkBuffer handle() const { return m_buffer; }

  private:
    VkBuffer m_buffer{};
    VkDeviceMemory m_memory{};

    uint32_t m_count;

    std::shared_ptr<VulkanDevice> m_device;
};

//
// Uniform Buffer
//

template <typename T>
class VulkanUniformBuffer {
  public:
    explicit VulkanUniformBuffer(std::shared_ptr<VulkanDevice> device) : m_device(std::move(device)) {
        m_size = sizeof(T);
        std::tie(m_buffer, m_memory) =
            BufferUtils::create_buffer(m_device,
                                       m_size,
                                       VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VK_CHECK(vkMapMemory(m_device->handle(), m_memory, 0, m_size, 0, &m_map_data))
    }

    ~VulkanUniformBuffer() {
        vkUnmapMemory(m_device->handle(), m_memory);

        vkDestroyBuffer(m_device->handle(), m_buffer, nullptr);
        vkFreeMemory(m_device->handle(), m_memory, nullptr);
    }

    void update(const T& data) { memcpy(m_map_data, &data, m_size); }

    [[nodiscard]] uint32_t size() const { return m_size; }
    [[nodiscard]] VkBuffer handle() const { return m_buffer; }

  private:
    VkBuffer m_buffer{};
    VkDeviceMemory m_memory{};

    void* m_map_data{nullptr};
    uint32_t m_size;

    std::shared_ptr<VulkanDevice> m_device;
};