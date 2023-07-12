#include "cubemap.h"

#include "renderer/backend/renderer.h"

#include "renderer/backend/vulkan/vulkan_cubemap.h"
#include "renderer/backend/vulkan/vulkan_image.h"

namespace Phos {

std::shared_ptr<Cubemap> Cubemap::create(const Faces& faces) {
    switch (Renderer::graphics_api()) {
    case GraphicsAPI::Vulkan:
        return std::dynamic_pointer_cast<Cubemap>(std::make_shared<VulkanCubemap>(faces));
    default:
        PS_FAIL("Vulkan is the only supported api")
    }
}

std::shared_ptr<Cubemap> Cubemap::create(const Faces& faces, const std::string& directory) {
    switch (Renderer::graphics_api()) {
    case GraphicsAPI::Vulkan:
        return std::dynamic_pointer_cast<Cubemap>(std::make_shared<VulkanCubemap>(faces, directory));
    default:
        PS_FAIL("Vulkan is the only supported api")
    }
}

} // namespace Phos
