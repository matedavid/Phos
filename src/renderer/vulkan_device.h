#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>
#include <string_view>
#include <functional>

#include "renderer/vulkan_physical_device.h"
#include "renderer/vulkan_queue.h"
#include "renderer/vulkan_command_pool.h"

// Forward declarations
class VulkanInstance;
class VulkanCommandBuffer;

class VulkanDevice {
  public:
    VulkanDevice(const std::unique_ptr<VulkanInstance>& instance,
                 const VulkanPhysicalDevice::Requirements& requirements);
    ~VulkanDevice();

    [[nodiscard]] std::shared_ptr<VulkanCommandBuffer> create_command_buffer(VulkanQueue::Type type) const;
    void free_command_buffer(const std::shared_ptr<VulkanCommandBuffer>& command_buffer, VulkanQueue::Type type) const;

    void single_time_command_buffer(VulkanQueue::Type type,
                                    const std::function<void(const VulkanCommandBuffer&)>& func) const;

    [[nodiscard]] std::shared_ptr<VulkanQueue> get_graphics_queue() const {
        CORE_ASSERT(m_graphics_queue != nullptr, "Graphics queue was not requested")
        return m_graphics_queue;
    }
    [[nodiscard]] std::shared_ptr<VulkanQueue> get_presentation_queue() const {
        CORE_ASSERT(m_presentation_queue != nullptr, "Presentation queue was not requested")
        return m_presentation_queue;
    }

    [[nodiscard]] VkDevice handle() const { return m_device; }
    [[nodiscard]] VulkanPhysicalDevice physical_device() const { return m_physical_device; }

  private:
    VkDevice m_device{};
    VulkanPhysicalDevice m_physical_device;

    std::shared_ptr<VulkanQueue> m_graphics_queue = nullptr;
    std::shared_ptr<VulkanQueue> m_presentation_queue = nullptr;

    std::unique_ptr<VulkanCommandPool> m_graphics_command_pool = nullptr;

    [[nodiscard]] const std::shared_ptr<VulkanQueue>& get_queue_from_type(VulkanQueue::Type type) const;

    [[nodiscard]] VulkanPhysicalDevice select_physical_device(const std::unique_ptr<VulkanInstance>& instance,
                                                              const VulkanPhysicalDevice::Requirements& reqs) const;
};
