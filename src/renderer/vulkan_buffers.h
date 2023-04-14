#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <optional>

#include "renderer/vulkan_device.h"
#include "renderer/vulkan_command_buffer.h"
#include "renderer/vulkan_buffer.h"

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

#include <iostream>
//
// Vertex Buffer
//
template <typename T>
class VulkanVertexBuffer {
  public:
    VulkanVertexBuffer(std::shared_ptr<VulkanDevice> device, const std::vector<T>& data) : m_device(std::move(device)) {
        const VkDeviceSize size = data.size() * sizeof(T);

        const auto staging_buffer = VulkanBuffer{
            m_device,
            size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        staging_buffer.copy_data(data.data());

        m_buffer = std::make_unique<VulkanBuffer>(m_device,
                                                  size,
                                                  VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        staging_buffer.copy_to_buffer(*m_buffer);

        m_size = (uint32_t)data.size();
    }

    ~VulkanVertexBuffer() = default;

    void bind(const std::shared_ptr<VulkanCommandBuffer>& command_buffer) const {
        std::array<VkBuffer, 1> vertex_buffers = {m_buffer->handle()};
        VkDeviceSize offsets[] = {0};

        vkCmdBindVertexBuffers(command_buffer->handle(), 0, vertex_buffers.size(), vertex_buffers.data(), offsets);
    }

    [[nodiscard]] uint32_t get_size() const { return m_size; }

    [[nodiscard]] VkBuffer handle() const { return m_buffer->handle(); }

  private:
    std::unique_ptr<VulkanBuffer> m_buffer;
    uint32_t m_size;

    std::shared_ptr<VulkanDevice> m_device;
};

//
// Index Buffer
//
class VulkanIndexBuffer {
  public:
    VulkanIndexBuffer(std::shared_ptr<VulkanDevice> device, const std::vector<uint32_t>& indices);
    ~VulkanIndexBuffer() = default;

    void bind(const std::shared_ptr<VulkanCommandBuffer>& command_buffer) const;
    [[nodiscard]] uint32_t get_count() const { return m_count; }

    [[nodiscard]] VkBuffer handle() const { return m_buffer->handle(); }

  private:
    std::unique_ptr<VulkanBuffer> m_buffer;
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
        m_buffer =
            std::make_unique<VulkanBuffer>(m_device,
                                           m_size,
                                           VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        m_buffer->map_memory(m_map_data);
    }

    ~VulkanUniformBuffer() { m_buffer->unmap_memory(); }

    void update(const T& data) { memcpy(m_map_data, &data, m_size); }

    [[nodiscard]] uint32_t size() const { return m_size; }
    [[nodiscard]] VkBuffer handle() const { return m_buffer->handle(); }

  private:
    std::unique_ptr<VulkanBuffer> m_buffer;

    void* m_map_data{nullptr};
    uint32_t m_size;

    std::shared_ptr<VulkanDevice> m_device;
};