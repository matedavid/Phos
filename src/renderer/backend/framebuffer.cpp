#include "framebuffer.h"

#include "utility/logging.h"

#include "renderer/backend/renderer.h"

#include "renderer/backend/vulkan/vulkan_framebuffer.h"

namespace Phos {

std::shared_ptr<Framebuffer> Framebuffer::create(const Description& description) {
    switch (Renderer::graphics_api()) {
    case GraphicsAPI::Vulkan:
        return std::dynamic_pointer_cast<Framebuffer>(std::make_shared<VulkanFramebuffer>(description));
    default:
        PHOS_FAIL("Vulkan is the only supported api");
    }
}

} // namespace Phos
