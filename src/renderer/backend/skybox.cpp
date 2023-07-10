#include "skybox.h"

#include "renderer/backend/renderer.h"

#include "renderer/backend/vulkan/vulkan_skybox.h"
#include "renderer/backend/vulkan/vulkan_image.h"

namespace Phos {

std::shared_ptr<Skybox> Skybox::create(const Sides& sides) {
    switch (Renderer::graphics_api()) {
    case GraphicsAPI::Vulkan:
        return std::dynamic_pointer_cast<Skybox>(std::make_shared<VulkanSkybox>(sides));
    default:
        PS_FAIL("Vulkan is the only supported api")
    }
}

std::shared_ptr<Skybox> Skybox::create(const Sides& sides, const std::string& directory) {
    switch (Renderer::graphics_api()) {
    case GraphicsAPI::Vulkan:
        return std::dynamic_pointer_cast<Skybox>(std::make_shared<VulkanSkybox>(sides, directory));
    default:
        PS_FAIL("Vulkan is the only supported api")
    }
}

} // namespace Phos
