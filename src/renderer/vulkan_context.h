#pragma once

#include "vk_core.h"

#include "renderer/vulkan_instance.h"
#include "renderer/vulkan_physical_device.h"
#include "renderer/vulkan_swapchain.h"
#include "renderer/vulkan_device.h"
#include "renderer/vulkan_render_pass.h"
#include "renderer/vulkan_graphics_pipeline.h"
#include "renderer/vulkan_framebuffer.h"
#include "renderer/vulkan_command_buffer.h"
#include "renderer/vulkan_command_pool.h"
#include "renderer/vulkan_buffers.h"

// Forward declarations
class Window;

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

class VulkanContext {
  public:
    explicit VulkanContext(const std::shared_ptr<Window>& window);
    ~VulkanContext();

    void update();

  private:
    std::unique_ptr<VulkanInstance> m_instance;
    std::shared_ptr<VulkanSwapchain> m_swapchain;
    std::shared_ptr<VulkanDevice> m_device;
    std::shared_ptr<VulkanRenderPass> m_render_pass;
    std::shared_ptr<VulkanGraphicsPipeline> m_pipeline;

    std::shared_ptr<VulkanCommandPool> m_command_pool;
    std::shared_ptr<VulkanCommandBuffer> m_command_buffer;
    std::vector<std::shared_ptr<VulkanFramebuffer>> m_present_framebuffers;

    std::unique_ptr<VulkanVertexBuffer> m_vertex_buffer;
    std::unique_ptr<VulkanIndexBuffer> m_index_buffer;

    VkSemaphore image_available_semaphore;
    VkSemaphore render_finished_semaphore;
    VkFence in_flight_fence;

    VkQueue m_graphics_queue;

    [[nodiscard]] VulkanPhysicalDevice select_physical_device(const std::vector<VulkanPhysicalDevice>& physical_devices,
                                                              const std::vector<const char*>& extensions) const;
};
