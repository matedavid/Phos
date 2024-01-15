#include "vulkan_physical_device.h"

#include <algorithm>
#include <string>

#include "vk_core.h"
#include "utility/logging.h"

namespace Phos {

VulkanPhysicalDevice::VulkanPhysicalDevice(VkPhysicalDevice physical_device) : m_physical_device(physical_device) {}

bool VulkanPhysicalDevice::is_suitable(const Requirements& requirements) const {
    // Supports extensions
    const auto extension_properties = get_extension_properties();

    for (const auto& extension : requirements.extensions) {
        const bool contained = std::ranges::any_of(extension_properties, [&extension](const auto& property) {
            return std::string_view(property.extensionName) == extension;
        });

        if (!contained)
            return false;
    }

    // Contains queue families
    PHOS_ASSERT(!requirements.presentation || requirements.surface != VK_NULL_HANDLE,
                "Surface required to test presentation queue support");

    bool graphics = !requirements.graphics;
    bool compute = !requirements.compute;
    bool transfer = !requirements.transfer;
    bool presentation = !requirements.presentation;

    const auto queue_family_properties = get_queue_family_properties();

    for (uint32_t idx = 0; idx < queue_family_properties.size(); ++idx) {
        const auto& property = queue_family_properties[idx];

        // Supports graphics queue
        if (property.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            graphics = true;

        // Supports compute queue
        if (property.queueFlags & VK_QUEUE_COMPUTE_BIT)
            compute = true;

        // Supports transfer queue
        if (property.queueFlags & VK_QUEUE_TRANSFER_BIT)
            transfer = true;

        // Supports presentation queue
        VkBool32 presentation_supported;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(
            m_physical_device, idx, requirements.surface, &presentation_supported));

        if (presentation_supported)
            presentation = true;
    }

    return graphics && compute && transfer && presentation;
}

VulkanPhysicalDevice::QueueFamilies VulkanPhysicalDevice::get_queue_families(const Requirements& requirements) const {
    QueueFamilies queue_families{};

    const auto queue_family_properties = get_queue_family_properties();

    for (uint32_t idx = 0; idx < queue_family_properties.size(); ++idx) {
        const auto& property = queue_family_properties[idx];

        if (requirements.graphics && (property.queueFlags & VK_QUEUE_GRAPHICS_BIT))
            queue_families.graphics = idx;

        if (requirements.compute && (property.queueFlags & VK_QUEUE_COMPUTE_BIT))
            queue_families.compute = idx;

        if (requirements.transfer && (property.queueFlags & VK_QUEUE_TRANSFER_BIT))
            queue_families.transfer = idx;

        if (requirements.presentation) {
            VkBool32 presentation_supported;
            VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(
                m_physical_device, idx, requirements.surface, &presentation_supported));

            if (presentation_supported)
                queue_families.presentation = idx;
        }
    }

    return queue_families;
}

std::optional<uint32_t> VulkanPhysicalDevice::find_memory_type(uint32_t filter,
                                                               VkMemoryPropertyFlags properties) const {
    const auto memory_properties = get_memory_properties();

    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i) {
        if (filter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    return {};
}

std::vector<VkExtensionProperties> VulkanPhysicalDevice::get_extension_properties() const {
    uint32_t extension_properties_count;
    VK_CHECK(vkEnumerateDeviceExtensionProperties(m_physical_device, nullptr, &extension_properties_count, nullptr));

    std::vector<VkExtensionProperties> extension_properties(extension_properties_count);
    VK_CHECK(vkEnumerateDeviceExtensionProperties(
        m_physical_device, nullptr, &extension_properties_count, extension_properties.data()));

    return extension_properties;
}

std::vector<VkQueueFamilyProperties> VulkanPhysicalDevice::get_queue_family_properties() const {
    uint32_t queue_family_properties_count;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queue_family_properties_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_properties_count);
    vkGetPhysicalDeviceQueueFamilyProperties(
        m_physical_device, &queue_family_properties_count, queue_family_properties.data());

    return queue_family_properties;
}

VkPhysicalDeviceProperties VulkanPhysicalDevice::get_properties() const {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_physical_device, &properties);

    return properties;
}

VkPhysicalDeviceMemoryProperties VulkanPhysicalDevice::get_memory_properties() const {
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(m_physical_device, &memory_properties);

    return memory_properties;
}

} // namespace Phos
