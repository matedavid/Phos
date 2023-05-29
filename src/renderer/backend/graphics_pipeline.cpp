#include "graphics_pipeline.h"

#include "renderer/backend/vulkan/vulkan_graphics_pipeline.h"

namespace Phos {

std::shared_ptr<GraphicsPipeline> GraphicsPipeline::create(const Phos::GraphicsPipeline::Description& description) {
    return std::dynamic_pointer_cast<GraphicsPipeline>(std::make_shared<VulkanGraphicsPipeline>(description));
}

} // namespace Phos
