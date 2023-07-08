#include "material.h"

#include "renderer/backend/renderer.h"

#include "renderer/backend/vulkan/vulkan_material.h"

namespace Phos {

std::shared_ptr<Material> Material::create(const std::shared_ptr<Shader>& shader, const std::string& name) {
    switch (Renderer::graphics_api()) {
    case GraphicsAPI::Vulkan:
        return std::dynamic_pointer_cast<Material>(std::make_shared<VulkanMaterial>(shader, name));
    default:
        PS_FAIL("Vulkan is the only supported api")
    }
}

} // namespace Phos
