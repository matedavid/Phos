#pragma once

#include <vulkan/vulkan.h>

namespace Phos {

// Forward declarations
class VulkanDevice;
class VulkanImage;

class VulkanBuffer {
  public:
    VulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    ~VulkanBuffer();

    void map_memory(void*& memory) const;
    void unmap_memory() const;

    void copy_data(const void* data) const;
    void copy_to_buffer(const VulkanBuffer& buffer) const;
    void copy_to_image(const VulkanImage& image) const;

    [[nodiscard]] VkBuffer handle() const { return m_buffer; }

  private:
    VkBuffer m_buffer{};
    VkDeviceMemory m_memory{};

    VkDeviceSize m_size;
};

} // namespace Phos
