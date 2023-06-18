#pragma once

#include "core.h"

namespace Phos {

class CommandBuffer {
  public:
    virtual ~CommandBuffer() = default;

    static std::shared_ptr<CommandBuffer> create();

    virtual void record(const std::function<void(void)>& func) const = 0;
};

} // namespace Phos
