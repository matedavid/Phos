#include "image.h"

#include "renderer/backend/renderer.h"
#include "renderer/backend/vulkan/vulkan_image.h"

namespace Phos {

std::shared_ptr<Image> Image::create(const Phos::Image::Description& description) {
    switch (Renderer::graphics_api()) {
    case GraphicsAPI::Vulkan:
        return std::dynamic_pointer_cast<Image>(std::make_shared<VulkanImage>(description));
    default:
        PS_FAIL("Vulkan is the only supported api")
    }
}

} // namespace Phos
