#pragma once

namespace Phos {

class Layer {
  public:
    virtual ~Layer() = default;

    virtual void on_update([[maybe_unused]] double ts) = 0;
};

} // namespace Phos