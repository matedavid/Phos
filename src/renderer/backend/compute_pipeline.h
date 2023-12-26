#pragma once

#include <memory>
#include <functional>

#include <glm/glm.hpp>

namespace Phos {

// Forward declarations
class Shader;
class CommandBuffer;
class Texture;

class ComputePipelineStepBuilder {
  public:
    virtual ~ComputePipelineStepBuilder() = default;

    virtual void set_push_constants(std::string_view name, uint32_t size, const void* data) = 0;

    template <typename T>
    void set_push_constants(std::string_view name, const T& data) {
        set_push_constants(name, sizeof(T), &data);
    }

    virtual void set(std::string_view name, const std::shared_ptr<Texture>& texture) = 0;
    virtual void set(std::string_view name, const std::shared_ptr<Texture>& texture, uint32_t mip_level) = 0;
};

class ComputePipeline {
  public:
    struct Description {
        std::shared_ptr<Shader> shader;
    };

    virtual ~ComputePipeline() = default;

    [[nodiscard]] static std::shared_ptr<ComputePipeline> create(const Description& description);

    using StepBuilder = ComputePipelineStepBuilder;

    virtual void add_step(const std::function<void(StepBuilder&)>& func, glm::uvec3 work_groups) = 0;
    virtual void execute(const std::shared_ptr<CommandBuffer>& command_buffer) = 0;
};

} // namespace Phos
