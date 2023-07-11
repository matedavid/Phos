#include "skybox.h"

#include "renderer/backend/renderer.h"

#include "renderer/backend/vulkan/vulkan_skybox.h"
#include "renderer/backend/vulkan/vulkan_image.h"

namespace Phos {

std::shared_ptr<Skybox> Skybox::create(const Faces& faces) {
    switch (Renderer::graphics_api()) {
    case GraphicsAPI::Vulkan:
        return std::dynamic_pointer_cast<Skybox>(std::make_shared<VulkanSkybox>(faces));
    default:
        PS_FAIL("Vulkan is the only supported api")
    }
}

std::shared_ptr<Skybox> Skybox::create(const Faces& faces, const std::string& directory) {
    switch (Renderer::graphics_api()) {
    case GraphicsAPI::Vulkan:
        return std::dynamic_pointer_cast<Skybox>(std::make_shared<VulkanSkybox>(faces, directory));
    default:
        PS_FAIL("Vulkan is the only supported api")
    }
}

} // namespace Phos
