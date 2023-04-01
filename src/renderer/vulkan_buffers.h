#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <optional>

// Forward declarations
class VulkanDevice;
class VulkanCommandBuffer;

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

    [[nodiscard]] std::optional<uint32_t> find_memory_type(uint32_t filter, VkMemoryPropertyFlags properties) const;
};
