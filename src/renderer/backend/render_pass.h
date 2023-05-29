#pragma once

#include "core.h"

namespace Phos {

// Forward declarations
class Framebuffer;

class RenderPass {
  public:
    struct Description {
        std::string debug_name;

        bool presentation_target = false; // Is render pass used in presentation pass
        std::shared_ptr<Framebuffer> target_framebuffer = nullptr;
    };

    virtual ~RenderPass() = default;

    static std::shared_ptr<RenderPass> create(Description description);
};

} // namespace Phos
