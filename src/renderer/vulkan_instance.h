#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <vector>

// Forward declarations
class VulkanPhysicalDevice;

class VulkanInstance {
  public:
    explicit VulkanInstance(const std::vector<const char*>& required_extensions);
    ~VulkanInstance();

    [[nodiscard]] std::vector<VulkanPhysicalDevice> get_physical_devices() const;

    [[nodiscard]] VkInstance handle() const { return m_instance; }

  private:
    VkInstance m_instance{};

    bool validation_layers_available(const std::vector<const char*>& validation_layers);
};
