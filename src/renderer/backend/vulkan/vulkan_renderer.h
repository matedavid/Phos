#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "renderer/backend/renderer.h"

namespace Phos {

// Forward declarations
class VulkanUniformBuffer;
class VulkanVertexBuffer;
class VulkanIndexBuffer;
class VulkanSwapchain;
class VulkanQueue;
class VulkanDescriptorAllocator;

class VulkanRenderer : public INativeRenderer {
  public:
    explicit VulkanRenderer(const RendererConfig& config);
    ~VulkanRenderer() override;
    void wait_idle() override;

    void begin_frame(const FrameInformation& info) override;
    void end_frame() override;

    void bind_graphics_pipeline(const std::shared_ptr<CommandBuffer>& command_buffer,
                                const std::shared_ptr<GraphicsPipeline>& pipeline) override;

    void bind_push_constant(const std::shared_ptr<CommandBuffer>& command_buffer,
                            const std::shared_ptr<GraphicsPipeline>& pipeline,
                            uint32_t size,
                            const void* data) override;

    void submit_static_mesh(const std::shared_ptr<CommandBuffer>& command_buffer,
                            const std::shared_ptr<StaticMesh>& mesh) override;

    void begin_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                           const std::shared_ptr<RenderPass>& render_pass) override;

    void end_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                         const std::shared_ptr<RenderPass>& render_pass) override;

    void record_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                            const std::shared_ptr<RenderPass>& render_pass,
                            const std::function<void(void)>& func) override;

    void submit_command_buffer(const std::shared_ptr<CommandBuffer>& command_buffer) override;

    void draw_screen_quad(const std::shared_ptr<CommandBuffer>& command_buffer) override;

    std::shared_ptr<Framebuffer> current_frame_framebuffer() override;
    std::shared_ptr<Framebuffer> presentation_framebuffer() override;

  private:
    std::shared_ptr<VulkanSwapchain> m_swapchain;

    std::shared_ptr<VulkanQueue> m_graphics_queue;
    std::shared_ptr<VulkanQueue> m_presentation_queue;

    VkSemaphore image_available_semaphore{VK_NULL_HANDLE};
    VkSemaphore render_finished_semaphore{VK_NULL_HANDLE};
    VkFence in_flight_fence{VK_NULL_HANDLE};

    // Frame descriptors
    struct CameraUniformBuffer {
        glm::mat4 projection;
        glm::mat4 view;
        // glm::mat4 view_projection;
        glm::vec3 position;
    };

    std::shared_ptr<VulkanDescriptorAllocator> m_allocator;

    std::shared_ptr<VulkanUniformBuffer> m_camera_ubo;
    std::shared_ptr<VulkanUniformBuffer> m_lights_ubo;

    VkDescriptorSet m_frame_descriptor_set{VK_NULL_HANDLE};

    // Screen quad info
    struct ScreenQuadVertex {
        glm::vec3 position;
        glm::vec2 texture_coord;
    };

    std::shared_ptr<VulkanVertexBuffer> m_screen_quad_vertex;
    std::shared_ptr<VulkanIndexBuffer> m_screen_quad_index;
};

} // namespace Phos