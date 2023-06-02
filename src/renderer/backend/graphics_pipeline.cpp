#include "graphics_pipeline.h"

#include "renderer/backend/renderer.h"
#include "renderer/backend/vulkan/vulkan_graphics_pipeline.h"

namespace Phos {

std::shared_ptr<GraphicsPipeline> GraphicsPipeline::create(const Phos::GraphicsPipeline::Description& description) {
    switch (Renderer::graphics_api()) {
    case GraphicsAPI::Vulkan:
        return std::dynamic_pointer_cast<GraphicsPipeline>(std::make_shared<VulkanGraphicsPipeline>(description));
    default:
        PS_FAIL("Vulkan is the only supported api")
    }
}

} // namespace Phos
