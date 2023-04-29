#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <vector>

namespace Phos {

// Forward declarations
class VulkanPhysicalDevice;
class Window;

class VulkanInstance {
  public:
    explicit VulkanInstance(const std::shared_ptr<Window>& window);
    ~VulkanInstance();

    [[nodiscard]] std::vector<VulkanPhysicalDevice> get_physical_devices() const;
    [[nodiscard]] VkSurfaceKHR get_surface() const { return m_surface; };

    [[nodiscard]] VkInstance handle() const { return m_instance; }

  private:
    VkInstance m_instance{};
    VkSurfaceKHR m_surface{};

    bool validation_layers_available(const std::vector<const char*>& validation_layers);
};

} // namespace Phos
