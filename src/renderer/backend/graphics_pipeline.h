#pragma once

#include <memory>
#include <vector>

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

struct Viewport {
    float x{}, y{};
    float width{}, height{};
    float minDepth = 0.0f, maxDepth = 1.0f;
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

    virtual void set_viewport(const std::shared_ptr<CommandBuffer>& command_buffer, const Viewport& viewport) const = 0;

    virtual void bind_push_constants(const std::shared_ptr<CommandBuffer>& command_buffer,
                                     std::string_view name,
                                     uint32_t size,
                                     const void* data) = 0;

    template <typename T>
    void bind_push_constants(const std::shared_ptr<CommandBuffer>& command_buffer,
                             std::string_view name,
                             const T& data) {
        bind_push_constants(command_buffer, name, sizeof(T), &data);
    }

    virtual void add_input(std::string_view name, const std::shared_ptr<UniformBuffer>& ubo) = 0;
    virtual void add_input(std::string_view name, const std::shared_ptr<Texture>& texture) = 0;
    virtual void add_input(std::string_view name, const std::vector<std::shared_ptr<Texture>>& textures) = 0;
    virtual void add_input(std::string_view name, const std::shared_ptr<Cubemap>& cubemap) = 0;

    virtual void update_input(std::string_view name, const std::shared_ptr<Texture>& texture) = 0;
};

} // namespace Phos
