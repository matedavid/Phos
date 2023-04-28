#pragma once

#include "vk_core.h"

#include <glm/glm.hpp>
#include <memory>

// Forward declarations
class VulkanInstance;
class VulkanDevice;
class Window;

class VulkanContext {
  public:
    static std::unique_ptr<VulkanInstance> instance;
    static std::shared_ptr<VulkanDevice> device;

    static void init(const std::shared_ptr<Window>& window);
    static void free();
};
