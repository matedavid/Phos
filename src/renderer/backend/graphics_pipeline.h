#pragma once

#include "core.h"

namespace Phos {

// Forward declarations
class Shader;
class Framebuffer;
class CommandBuffer;
class UniformBuffer;
class Texture;

class GraphicsPipeline {
  public:
    struct Description {
        std::shared_ptr<Shader> shader;
        std::shared_ptr<Framebuffer> target_framebuffer;
    };

    virtual ~GraphicsPipeline() = default;

    static std::shared_ptr<GraphicsPipeline> create(const Description& description);

    virtual void bind(const std::shared_ptr<CommandBuffer>& command_buffer) = 0;
    [[nodiscard]] virtual bool bake() = 0;

    virtual void add_input(std::string_view name, const std::shared_ptr<UniformBuffer>& ubo) = 0;
    virtual void add_input(std::string_view name, const std::shared_ptr<Texture>& texture) = 0;
};

} // namespace Phos
