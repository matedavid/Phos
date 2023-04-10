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

    static void copy_buffer(const std::shared_ptr<VulkanDevice>& device,
                            VkBuffer source,
                            VkBuffer dest,
                            VkDeviceSize size) {
        // TODO: Maybe should be transfer queue instead of graphics
        const auto command_buffer = device->create_command_buffer(VulkanQueue::Type::Graphics);
        const auto graphics_queue = device->get_graphics_queue();

        command_buffer->begin(true);

        VkBufferCopy copy{};
        copy.srcOffset = 0;
        copy.dstOffset = 0;
        copy.size = size;

        vkCmdCopyBuffer(command_buffer->handle(), source, dest, 1, &copy);

        command_buffer->end();

        // Submit queue
        const std::array<VkCommandBuffer, 1> command_buffers = {command_buffer->handle()};

        VkSubmitInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.commandBufferCount = 1;
        info.pCommandBuffers = command_buffers.data();

        graphics_queue->submit(info, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphics_queue->handle());

        // Free command buffer
        // TODO: Maybe should be transfer queue instead of graphics
        device->free_command_buffer(command_buffer, VulkanQueue::Type::Graphics);
    }

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

        const auto& [staging_buffer, staging_buffer_memory] =
            BufferUtils::create_buffer(m_device,
                                       size,
                                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void* map_data;
        VK_CHECK(vkMapMemory(m_device->handle(), staging_buffer_memory, 0, size, 0, &map_data))
        memcpy(map_data, data.data(), (size_t)size);
        vkUnmapMemory(m_device->handle(), staging_buffer_memory);

        std::tie(m_buffer, m_memory) =
            BufferUtils::create_buffer(m_device,
                                       size,
                                       VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        BufferUtils::copy_buffer(m_device, staging_buffer, m_buffer, size);

        // Destroy staging buffer
        vkDestroyBuffer(m_device->handle(), staging_buffer, nullptr);
        vkFreeMemory(m_device->handle(), staging_buffer_memory, nullptr);

        m_size = (uint32_t)data.size();
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