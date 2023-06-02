#pragma once

#include "renderer/backend/renderer.h"

namespace Phos {

class VulkanRenderer : public INativeRenderer {
  public:
    explicit VulkanRenderer(const RendererConfig& config);
    ~VulkanRenderer() override;

    void submit_static_mesh(const std::shared_ptr<CommandBuffer>& command_buffer,
                            const std::shared_ptr<StaticMesh>& mesh) override;

    void begin_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                           const std::shared_ptr<RenderPass>& render_pass) override;

    void end_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                         const std::shared_ptr<RenderPass>& render_pass) override;

    void record_render_pass(const std::shared_ptr<CommandBuffer>& command_buffer,
                            const std::shared_ptr<RenderPass>& render_pass,
                            const std::function<void(void)>& func) override;
};

} // namespace Phos