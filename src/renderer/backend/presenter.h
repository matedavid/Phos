#pragma once

#include <memory>

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
    virtual void window_resized(uint32_t width, uint32_t height) = 0;
};

} // namespace Phos
