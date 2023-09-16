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
    virtual void execute(const std::shared_ptr<CommandBuffer>& command_buffer, glm::ivec3 work_groups) = 0;

    [[nodiscard]] virtual bool bake() = 0;
    virtual void invalidate() = 0;

    virtual void set(std::string_view name, const std::shared_ptr<Texture>& texture) = 0;
    virtual void set(std::string_view name, const std::shared_ptr<Texture>& texture, uint32_t mip_level) = 0;
};

} // namespace Phos
