#include "compute_pipeline.h"

#include "utility/logging.h"

#include "renderer/backend/renderer.h"

#include "renderer/backend/vulkan/vulkan_compute_pipeline.h"

namespace Phos {

std::shared_ptr<ComputePipeline> ComputePipeline::create(const Description& description) {
    switch (Renderer::graphics_api()) {
    case GraphicsAPI::Vulkan:
        return std::dynamic_pointer_cast<ComputePipeline>(std::make_shared<VulkanComputePipeline>(description));
    default:
        PHOS_FAIL("Vulkan is the only supported api");
    }
}

} // namespace Phos
