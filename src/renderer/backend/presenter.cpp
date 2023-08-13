#include "presenter.h"

#include "renderer/backend/renderer.h"
#include "renderer/backend/vulkan/vulkan_presenter.h"

namespace Phos {

std::shared_ptr<Presenter> Presenter::create(const std::shared_ptr<ISceneRenderer>& renderer,
                                             const std::shared_ptr<Window>& window) {
    switch (Renderer::graphics_api()) {
    case GraphicsAPI::Vulkan:
        return std::dynamic_pointer_cast<Presenter>(std::make_shared<VulkanPresenter>(renderer, window));
    default:
        PS_FAIL("Vulkan is the only supported api")
    }
}

} // namespace Phos
