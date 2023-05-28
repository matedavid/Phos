#include "framebuffer.h"

#include "renderer/backend/vulkan/vulkan_framebuffer.h"

namespace Phos {

std::shared_ptr<Framebuffer> Framebuffer::create(const Phos::Framebuffer::Description& description) {
    return std::dynamic_pointer_cast<Framebuffer>(std::make_shared<VulkanFramebuffer>(description));
}

} // namespace Phos
