#pragma once

#include "core.h"

namespace Phos {

// Forward declarations
class Framebuffer;
class CommandBuffer;

class RenderPass {
  public:
    struct Description {
        std::string debug_name;

        bool presentation_target = false; // Is render pass used in presentation pass
        std::shared_ptr<Framebuffer> target_framebuffer = nullptr;
    };

    virtual ~RenderPass() = default;

    static std::shared_ptr<RenderPass> create(Description description);

    virtual void begin(const std::shared_ptr<CommandBuffer>& command_buffer) = 0;
    virtual void begin(const std::shared_ptr<CommandBuffer>& command_buffer,
                       const std::shared_ptr<Framebuffer>& framebuffer) = 0;

    virtual void end(const std::shared_ptr<CommandBuffer>& command_buffer) = 0;
};

} // namespace Phos
