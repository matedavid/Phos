#pragma once

#include "vk_core.h"

#include <memory>

#define GLM_FORCE_RADIANS
#define GLM_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <core/window.h>
#include <input/input.h>

#include "renderer/backend/vulkan/vulkan_instance.h"
#include "renderer/backend/vulkan/vulkan_physical_device.h"
#include "renderer/backend/vulkan/vulkan_swapchain.h"
#include "renderer/backend/vulkan/vulkan_device.h"
#include "renderer/backend/vulkan/vulkan_render_pass.h"
#include "renderer/backend/vulkan/vulkan_graphics_pipeline.h"
#include "renderer/backend/vulkan/vulkan_framebuffer.h"
#include "renderer/backend/vulkan/vulkan_command_buffer.h"
#include "renderer/backend/vulkan/vulkan_command_pool.h"
#include "renderer/backend/vulkan/vulkan_buffers.h"
#include "renderer/backend/vulkan/vulkan_queue.h"
#include "renderer/backend/vulkan/vulkan_descriptors.h"
#include "renderer/backend/vulkan/vulkan_texture.h"
#include "renderer/backend/vulkan/vulkan_shader_module.h"

namespace Phos {

// Forward declarations
class Window;
class Model;

struct CameraUniformBuffer {
    glm::mat4 projection;
    glm::mat4 view;
};

struct ColorUniformBuffer {
    glm::vec4 color;
};

class VulkanRenderer {
  public:
    VulkanRenderer();
    ~VulkanRenderer();

    void update();

  private:
    std::shared_ptr<VulkanSwapchain> m_swapchain;
    std::shared_ptr<VulkanRenderPass> m_render_pass;
    std::shared_ptr<VulkanGraphicsPipeline> m_pipeline;

    std::shared_ptr<VulkanCommandBuffer> m_command_buffer;

    std::shared_ptr<Model> m_model;

    VkSemaphore image_available_semaphore;
    VkSemaphore render_finished_semaphore;
    VkFence in_flight_fence;

    std::shared_ptr<VulkanQueue> m_graphics_queue;
    std::shared_ptr<VulkanQueue> m_presentation_queue;

    std::shared_ptr<VulkanDescriptorAllocator> m_allocator;

    VkDescriptorSet m_vertex_shader_set{VK_NULL_HANDLE};
    VkDescriptorSet m_fragment_shader_set{VK_NULL_HANDLE};

    std::shared_ptr<VulkanUniformBuffer<ColorUniformBuffer>> m_color_ubo;
    std::shared_ptr<VulkanUniformBuffer<CameraUniformBuffer>> m_camera_ubo;


    struct CameraInfo {
        glm::vec3 position;
        glm::vec2 rotation;
        glm::vec2 mouse_pos;
    };

    CameraInfo m_camera_info{};

    void on_event(Event& event);
};

} // namespace Phos
