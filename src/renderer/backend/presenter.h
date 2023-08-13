#pragma once

#include "core.h"

namespace Phos {

// Forward declarations
class Window;
class ISceneRenderer;

class Presenter {
  public:
    virtual ~Presenter() = default;

    [[nodiscard]] static std::shared_ptr<Presenter> create(const std::shared_ptr<ISceneRenderer>& renderer,
                                                           const std::shared_ptr<Window>& window);

    virtual void present() = 0;
};

} // namespace Phos
