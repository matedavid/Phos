#pragma once

#include "core.h"

namespace Phos {

// Forward declarations
class Shader;
class Framebuffer;
class VulkanCommandBuffer;

class GraphicsPipeline {
  public:
    struct Description {
        std::shared_ptr<Shader> shader;
        std::shared_ptr<Framebuffer> target_framebuffer;
    };

    virtual ~GraphicsPipeline() = default;

    static std::shared_ptr<GraphicsPipeline> create(const Description& description);

    virtual void bind(const std::shared_ptr<VulkanCommandBuffer>& command_buffer) const = 0;
};

} // namespace Phos
