#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <vector>

class VulkanPhysicalDevice {
  public:
    struct Requirements {
        bool graphics = false;
        bool compute = false;
        bool transfer = false;

        bool presentation = false;
        VkSurfaceKHR surface = VK_NULL_HANDLE; // Only if presentation is true

        std::vector<const char*> extensions;
    };

    struct QueueFamilies {
        uint32_t graphics;
        uint32_t compute;
        uint32_t transfer;
        uint32_t presentation;
    };

    explicit VulkanPhysicalDevice(VkPhysicalDevice physical_device);
    ~VulkanPhysicalDevice() = default;

    [[nodiscard]] bool is_suitable(const Requirements& requirements) const;
    [[nodiscard]] QueueFamilies get_queue_families(const Requirements& requirements) const;

    [[nodiscard]] std::vector<VkExtensionProperties> get_extension_properties() const;
    [[nodiscard]] std::vector<VkQueueFamilyProperties> get_queue_family_properties() const;
    [[nodiscard]] VkPhysicalDeviceProperties get_properties() const;

    [[nodiscard]] VkPhysicalDevice handle() const { return m_physical_device; }

  private:
    VkPhysicalDevice m_physical_device;
};
