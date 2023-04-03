#include "vulkan_device.h"

#include <optional>
#include <ranges>
#include <algorithm>

#include "renderer/vulkan_instance.h"
#include "renderer/vulkan_queue.h"

VulkanDevice::VulkanDevice(const std::unique_ptr<VulkanInstance>& instance,
                           const VulkanPhysicalDevice::Requirements& requirements)
      : m_physical_device(select_physical_device(instance, requirements)) {
    fmt::print("Selected physical device: {}\n", m_physical_device.get_properties().deviceName);

    // Create queues
    const VulkanPhysicalDevice::QueueFamilies queue_families = m_physical_device.get_queue_families(requirements);

    std::vector<VkDeviceQueueCreateInfo> queues;

    VkDeviceQueueCreateInfo queue_create_info_base{};
    queue_create_info_base.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info_base.queueCount = 1;
    float priority = 1.0f;
    queue_create_info_base.pQueuePriorities = &priority;

    if (requirements.graphics) {
        queue_create_info_base.queueFamilyIndex = queue_families.graphics;
        queues.push_back(queue_create_info_base);
    }
    if (requirements.compute) {
        queue_create_info_base.queueFamilyIndex = queue_families.compute;
        queues.push_back(queue_create_info_base);
    }
    //    if (requirements.transfer) {
    //        queue_create_info_base.queueFamilyIndex = queue_families.transfer;
    //        queues.push_back(queue_create_info_base);
    //    }
    if (requirements.presentation && requirements.graphics != requirements.presentation) {
        queue_create_info_base.queueFamilyIndex = queue_families.presentation;
        queues.push_back(queue_create_info_base);
    }

    // Create device
    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = (uint32_t)queues.size();
    create_info.pQueueCreateInfos = queues.data();
    create_info.enabledExtensionCount = (uint32_t)requirements.extensions.size();

    std::vector<const char*> extensions;
    for (const auto& extension : requirements.extensions) {
        extensions.push_back(extension.data());
    }

    create_info.ppEnabledExtensionNames = extensions.data();

    VK_CHECK(vkCreateDevice(m_physical_device.handle(), &create_info, nullptr, &m_device));

    if (requirements.graphics) {
        VkQueue graphics_queue;
        vkGetDeviceQueue(m_device, queue_families.graphics, 0, &graphics_queue);

        m_graphics_queue = std::make_shared<VulkanQueue>(graphics_queue);
    }

    if (requirements.presentation) {
        // TODO: What if graphics queue is null pointer...
        if (queue_families.presentation == queue_families.graphics) m_presentation_queue = m_graphics_queue;
        else {
            VkQueue presentation_queue;
            vkGetDeviceQueue(m_device, queue_families.presentation, 0, &presentation_queue);

            m_presentation_queue = std::make_shared<VulkanQueue>(presentation_queue);
        }
    }
}

VulkanDevice::~VulkanDevice() {
    vkDestroyDevice(m_device, nullptr);
}

VulkanPhysicalDevice VulkanDevice::select_physical_device(const std::unique_ptr<VulkanInstance>& instance,
                                                          const VulkanPhysicalDevice::Requirements& reqs) const {
    const auto physical_devices = instance->get_physical_devices();

    const auto is_device_suitable = [&reqs](const VulkanPhysicalDevice& device) -> bool {
        return device.is_suitable(reqs);
    };

    // TODO: Should make these parameters configurable
    constexpr bool graphics_transfer_same_queue = true;
    constexpr bool graphics_presentation_same_queue = false;

    uint32_t max_score = 0;
    std::optional<VulkanPhysicalDevice> max_device;

    for (const auto& device : physical_devices | std::views::filter(is_device_suitable)) {
        uint32_t score = 0;

        const auto queue_families = device.get_queue_families(reqs);

        if (graphics_transfer_same_queue && queue_families.graphics == queue_families.transfer) score += 10;

        if (graphics_presentation_same_queue && queue_families.graphics == queue_families.presentation) score += 10;

        if (score > max_score) {
            max_device = device;
            max_score = score;
        }
    }

    CORE_ASSERT(max_device.has_value(), "There are no suitable devices")

    return max_device.value();
}
