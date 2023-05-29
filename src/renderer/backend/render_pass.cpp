#include "render_pass.h"

#include "renderer/backend/vulkan/vulkan_render_pass.h"

namespace Phos {

std::shared_ptr<RenderPass> RenderPass::create(Description description) {
    return std::dynamic_pointer_cast<RenderPass>(std::make_shared<VulkanRenderPass>(std::move(description)));
}

} // namespace Phos
