#include "render_pass.h"

#include "utility/logging.h"

#include "renderer/backend/renderer.h"

#include "renderer/backend/vulkan/vulkan_render_pass.h"

namespace Phos {

std::shared_ptr<RenderPass> RenderPass::create(Description description) {
    switch (Renderer::graphics_api()) {
    case GraphicsAPI::Vulkan:
        return std::dynamic_pointer_cast<RenderPass>(std::make_shared<VulkanRenderPass>(std::move(description)));
    default:
        PHOS_FAIL("Vulkan is the only supported api");
    }
}

} // namespace Phos
