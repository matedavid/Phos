#include "material.h"

#include "renderer/backend/renderer.h"

#include "renderer/backend/vulkan/vulkan_material.h"

namespace Phos {

std::shared_ptr<Material> Material::create(const Definition& definition) {
    switch (Renderer::graphics_api()) {
    case GraphicsAPI::Vulkan:
        return std::dynamic_pointer_cast<Material>(std::make_shared<VulkanMaterial>(definition));
    default:
        PS_FAIL("Vulkan is the only supported api")
    }
}

} // namespace Phos
