#include "image.h"

#include "renderer/backend/vulkan/vulkan_image.h"

namespace Phos {

std::shared_ptr<Image> Image::create(const Phos::Image::Description& description) {
    return std::dynamic_pointer_cast<Image>(std::make_shared<VulkanImage>(description));
}

} // namespace Phos
