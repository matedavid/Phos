#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <optional>

// Forward declarations
class VulkanDevice;
class VulkanCommandBuffer;

class BufferUtils {
  public:
    static std::pair<VkBuffer, VkDeviceMemory> create_buffer(
        const std::shared_ptr<VulkanDevice>& device,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties
    );

    static void copy_buffer(VkBuffer src, VkBuffer dest, VkDeviceSize size);

    static std::optional<uint32_t> find_memory_type(
        VkPhysicalDevice device,
        uint32_t filter,
        VkMemoryPropertyFlags properties
    );
};

//
// Vertex buffer
//
class VulkanVertexBuffer {
  public:
    VulkanVertexBuffer(std::shared_ptr<VulkanDevice> device, const std::vector<float>& data);
    ~VulkanVertexBuffer();

    void bind(const std::shared_ptr<VulkanCommandBuffer>& command_buffer) const;

    [[nodiscard]] VkBuffer handle() const { return m_buffer; }

  private:
    VkBuffer m_buffer{};
    VkDeviceMemory m_memory{};

    std::shared_ptr<VulkanDevice> m_device;
};

//
// Index buffer
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