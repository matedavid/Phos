#pragma once

#include "vk_core.h"

#include <glm/glm.hpp>
#include <memory>

namespace Phos {

// Forward declarations
class VulkanInstance;
class VulkanDevice;
class VulkanDescriptorLayoutCache;
class Window;

class VulkanContext {
  public:
    static std::unique_ptr<VulkanInstance> instance;
    static std::unique_ptr<VulkanDevice> device;
    static std::shared_ptr<VulkanDescriptorLayoutCache> descriptor_layout_cache;

    static void init(const std::shared_ptr<Window>& window);
    static void free();
};

} // namespace Phos
