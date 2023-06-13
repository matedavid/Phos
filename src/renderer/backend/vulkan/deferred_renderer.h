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
#include "renderer/backend/vulkan/vulkan_image.h"
#include "renderer/backend/vulkan/vulkan_shader.h"

namespace Phos {

// Forward declarations
class Window;
class StaticMesh;

struct CameraUniformBuffer {
    glm::mat4 projection;
    glm::mat4 view;
    glm::vec3 position;
};

struct LightsUniformBuffer {
    glm::vec4 positions[10];
    glm::vec4 colors[10];
    int count;
};

struct ModelInfoPushConstant {
    glm::mat4 model;
    glm::vec4 color;
};

struct DeferredVertex {
    glm::vec3 position;
    glm::vec2 texture_coord;
};

class DeferredRenderer {
  public:
    DeferredRenderer();
    ~DeferredRenderer();

    void update();

  private:
    std::shared_ptr<VulkanSwapchain> m_swapchain;
    std::shared_ptr<VulkanCommandBuffer> m_command_buffer;

    // Geometry pass
    std::shared_ptr<VulkanTexture> m_position_texture;
    std::shared_ptr<VulkanTexture> m_normal_texture;
    std::shared_ptr<VulkanTexture> m_color_specular_texture;

    std::shared_ptr<VulkanFramebuffer> m_geometry_framebuffer;

    std::shared_ptr<VulkanGraphicsPipeline> m_geometry_pipeline;
    std::shared_ptr<VulkanRenderPass> m_geometry_pass;

    // Lighting pass
    std::shared_ptr<VulkanVertexBuffer> m_quad_vertex;
    std::shared_ptr<VulkanIndexBuffer> m_quad_index;

    std::shared_ptr<VulkanGraphicsPipeline> m_lighting_pipeline;
    std::shared_ptr<VulkanRenderPass> m_lighting_pass;

    // Flat color pipeline
    std::shared_ptr<VulkanGraphicsPipeline> m_flat_color_pipeline;

    // Uniform buffers
    std::shared_ptr<VulkanUniformBuffer> m_camera_ubo;
    std::shared_ptr<VulkanUniformBuffer> m_lights_ubo;

    VkDescriptorSet m_camera_set;
    VkDescriptorSet m_lighting_fragment_set;

    // Models
    std::shared_ptr<StaticMesh> m_model;
    std::shared_ptr<StaticMesh> m_cube;

    VkSemaphore image_available_semaphore;
    VkSemaphore render_finished_semaphore;
    VkFence in_flight_fence;

    std::shared_ptr<VulkanQueue> m_graphics_queue;
    std::shared_ptr<VulkanQueue> m_presentation_queue;

    std::shared_ptr<VulkanDescriptorAllocator> m_allocator;

    struct CameraInfo {
        glm::vec3 position;
        glm::vec2 rotation;
        glm::vec2 mouse_pos;
    };

    CameraInfo m_camera_info{};

    LightsUniformBuffer light_info{};

    void on_event(Event& event);
    void update_light_info();
};

} // namespace Phos
