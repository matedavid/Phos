#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "renderer/backend/renderer.h"

namespace Phos {

// Forward declarations
template <typename T>
class VulkanUniformBuffer;

class VulkanRenderer : public INativeRenderer {
  public:
    explicit VulkanRenderer(const RendererConfig& config);
    ~VulkanRenderer() override;

    void begin_frame(const FrameInformation& info) override;
    void end_frame() override;

    void submit_static_mesh(const std::shared_ptr<CommandBuffer>& command_buffer,
                            const std::shared_ptr<StaticMesh>& mesh) override;

    void begin_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                           const std::shared_ptr<RenderPass>& render_pass) override;

    void end_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                         const std::shared_ptr<RenderPass>& render_pass) override;

    void record_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                            const std::shared_ptr<RenderPass>& render_pass,
                            const std::function<void(void)>& func) override;

  private:
    struct CameraUniformBuffer {
        glm::mat4 projection;
        glm::mat4 view;
        glm::mat4 view_projection;
        glm::vec3 position;
    };

    std::shared_ptr<VulkanUniformBuffer<CameraUniformBuffer>> m_camera_ubo;
    // std::shared_ptr<VulkanUniformBuffer<CameraUniformBuffer>> m_lights_ubo;

    VkDescriptorSet m_frame_descriptor_set{VK_NULL_HANDLE};
};

} // namespace Phos