#pragma once

#include "core.h"

namespace Phos {

// Forward declarations
class Shader;
class Framebuffer;
class CommandBuffer;
class UniformBuffer;
class Texture;
class Cubemap;

enum class FrontFace {
    Clockwise,
    CounterClockwise,
};

enum class DepthCompareOp {
    Less,
    LessEq,
};

class GraphicsPipeline {
  public:
    struct Description {
        std::shared_ptr<Shader> shader;
        std::shared_ptr<Framebuffer> target_framebuffer;

        // Resterization
        FrontFace front_face = FrontFace::CounterClockwise;

        // Depth test
        DepthCompareOp depth_compare_op = DepthCompareOp::Less;
        bool depth_write = true;
    };

    virtual ~GraphicsPipeline() = default;

    static std::shared_ptr<GraphicsPipeline> create(const Description& description);

    [[nodiscard]] virtual bool bake() = 0;
    [[nodiscard]] virtual std::shared_ptr<Framebuffer> target_framebuffer() const = 0;

    virtual void add_input(std::string_view name, const std::shared_ptr<UniformBuffer>& ubo) = 0;
    virtual void add_input(std::string_view name, const std::shared_ptr<Texture>& texture) = 0;
    virtual void add_input(std::string_view name, const std::shared_ptr<Cubemap>& cubemap) = 0;
};

} // namespace Phos
