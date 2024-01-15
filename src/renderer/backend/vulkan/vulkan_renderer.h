#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <array>

#include "renderer/backend/renderer.h"

namespace Phos {

// Forward declarations
class VulkanUniformBuffer;
class VulkanVertexBuffer;
class VulkanIndexBuffer;
class VulkanSwapchain;
class VulkanQueue;
class VulkanDescriptorAllocator;

constexpr uint32_t MAX_POINT_LIGHTS = 10;
constexpr uint32_t MAX_DIRECTIONAL_LIGHTS = 1;

class VulkanRenderer : public INativeRenderer {
  public:
    explicit VulkanRenderer(const RendererConfig& config);
    ~VulkanRenderer() override;
    void wait_idle() override;

    void begin_frame(const FrameInformation& info) override;
    void end_frame() override;

    void bind_graphics_pipeline(const std::shared_ptr<CommandBuffer>& command_buffer,
                                const std::shared_ptr<GraphicsPipeline>& pipeline) override;

    void submit_static_mesh(const std::shared_ptr<CommandBuffer>& command_buffer,
                            const std::shared_ptr<Mesh>& mesh,
                            const std::shared_ptr<Material>& material) override;

    void begin_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                           const std::shared_ptr<RenderPass>& render_pass) override;

    void end_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                         const std::shared_ptr<RenderPass>& render_pass) override;

    void submit_command_buffer(const std::shared_ptr<CommandBuffer>& command_buffer) override;

    void draw_screen_quad(const std::shared_ptr<CommandBuffer>& command_buffer) override;

    [[nodiscard]] uint32_t current_frame() override { return m_current_frame; }

  private:
    std::shared_ptr<VulkanQueue> m_graphics_queue;

    uint32_t m_current_frame = 0;

    std::vector<VkFence> m_in_flight_fences;

    // Frame descriptors
    struct CameraUniformBuffer {
        glm::mat4 projection;
        glm::mat4 view;
        // glm::mat4 view_projection;
        glm::vec3 position;
    };

    struct PointLightStruct {
        glm::vec4 color;
        glm::vec4 position;
    };

    struct DirectionalLightStruct {
        glm::vec4 color;
        glm::vec4 direction;
    };

    struct LightsUniformBuffer {
        std::array<PointLightStruct, MAX_POINT_LIGHTS> point_lights{};
        std::array<DirectionalLightStruct, MAX_DIRECTIONAL_LIGHTS> directional_lights{};

        uint32_t number_point_lights = 0;
        uint32_t number_directional_lights = 0;
    };

    std::shared_ptr<VulkanDescriptorAllocator> m_allocator;

    std::vector<std::shared_ptr<VulkanUniformBuffer>> m_camera_ubos;
    std::vector<std::shared_ptr<VulkanUniformBuffer>> m_lights_ubos;

    std::vector<VkDescriptorSet> m_frame_descriptor_sets;

    // Screen quad info
    struct ScreenQuadVertex {
        glm::vec3 position;
        glm::vec2 texture_coord;
    };

    std::shared_ptr<VulkanVertexBuffer> m_screen_quad_vertex;
    std::shared_ptr<VulkanIndexBuffer> m_screen_quad_index;
};

} // namespace Phos