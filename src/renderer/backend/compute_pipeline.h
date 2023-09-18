#pragma once

#include "core.h"

#include <glm/glm.hpp>

namespace Phos {

// Forward declarations
class Shader;
class CommandBuffer;
class Texture;

class ComputePipeline {
  public:
    struct Description {
        std::shared_ptr<Shader> shader;
    };

    virtual ~ComputePipeline() = default;

    [[nodiscard]] static std::shared_ptr<ComputePipeline> create(const Description& description);

    virtual void bind(const std::shared_ptr<CommandBuffer>& command_buffer) = 0;
    virtual void bind_push_constants(const std::shared_ptr<CommandBuffer>& command_buffer,
                                     std::string_view name,
                                     uint32_t size,
                                     const void* data) = 0;
    virtual void execute(const std::shared_ptr<CommandBuffer>& command_buffer, glm::ivec3 work_groups) = 0;

    template <typename T>
    void bind_push_constants(const std::shared_ptr<CommandBuffer>& command_buffer,
                             std::string_view name,
                             const T& data) {
        bind_push_constants(command_buffer, name, sizeof(T), &data);
    }

    [[nodiscard]] virtual bool bake() = 0;
    virtual void invalidate() = 0;

    virtual void set(std::string_view name, const std::shared_ptr<Texture>& texture) = 0;
    virtual void set(std::string_view name, const std::shared_ptr<Texture>& texture, uint32_t mip_level) = 0;
};

} // namespace Phos
