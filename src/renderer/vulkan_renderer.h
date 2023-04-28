#pragma once

#include "vk_core.h"

#include <memory>
#include <glm/glm.hpp>

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
#include "renderer/vulkan_queue.h"
#include "renderer/vulkan_descriptors.h"
#include "renderer/vulkan_texture.h"
#include "renderer/vulkan_shader_module.h"

// Forward declarations
class Window;

struct Vertex {
    glm::vec3 position;
    glm::vec2 texture_coords;
};

struct ColorUniformBuffer {
    glm::vec4 color;
};

class VulkanRenderer {
  public:
    explicit VulkanRenderer(const std::shared_ptr<Window>& window);
    ~VulkanRenderer();

    void update();

  private:
    std::shared_ptr<VulkanSwapchain> m_swapchain;
    std::shared_ptr<VulkanRenderPass> m_render_pass;
    std::shared_ptr<VulkanGraphicsPipeline> m_pipeline;

    std::shared_ptr<VulkanCommandBuffer> m_command_buffer;

    std::unique_ptr<VulkanVertexBuffer<Vertex>> m_vertex_buffer;
    std::unique_ptr<VulkanIndexBuffer> m_index_buffer;
    std::unique_ptr<VulkanTexture> m_texture;

    VkSemaphore image_available_semaphore;
    VkSemaphore render_finished_semaphore;
    VkFence in_flight_fence;

    std::shared_ptr<VulkanQueue> m_graphics_queue;
    std::shared_ptr<VulkanQueue> m_presentation_queue;

    std::shared_ptr<VulkanDescriptorLayoutCache> m_layout_cache;
    std::shared_ptr<VulkanDescriptorAllocator> m_allocator;

    VkDescriptorSet m_uniform_buffer_set{VK_NULL_HANDLE};
    std::shared_ptr<VulkanUniformBuffer<ColorUniformBuffer>> m_color_ubo;
};
