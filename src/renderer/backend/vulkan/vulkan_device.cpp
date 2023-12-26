#include "vulkan_device.h"

#include <optional>
#include <ranges>

#include "vk_core.h"

#include "renderer/backend/vulkan/vulkan_instance.h"
#include "renderer/backend/vulkan/vulkan_queue.h"

namespace Phos {

VulkanDevice::VulkanDevice(const std::unique_ptr<VulkanInstance>& instance,
                           const VulkanPhysicalDevice::Requirements& requirements)
      : m_physical_device(select_physical_device(instance, requirements)) {
    PHOS_LOG_INFO("Selected physical device: {}", m_physical_device.get_properties().deviceName);

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

    if (requirements.compute && queue_families.compute != queue_families.graphics) {
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
    extensions.reserve(requirements.extensions.size());
    for (const auto& extension : requirements.extensions) {
        extensions.push_back(extension.data());
    }

    create_info.ppEnabledExtensionNames = extensions.data();

    VK_CHECK(vkCreateDevice(m_physical_device.handle(), &create_info, nullptr, &m_device));

    // Request graphics queue
    if (requirements.graphics) {
        VkQueue graphics_queue;
        vkGetDeviceQueue(m_device, queue_families.graphics, 0, &graphics_queue);

        m_graphics_queue = std::make_shared<VulkanQueue>(graphics_queue, queue_families.graphics);
    }

    // Request presentation queue
    if (requirements.presentation) {
        // TODO: What if graphics queue is null pointer...
        if (queue_families.presentation == queue_families.graphics) {
            m_presentation_queue = m_graphics_queue;
        } else {
            VkQueue presentation_queue;
            vkGetDeviceQueue(m_device, queue_families.presentation, 0, &presentation_queue);

            m_presentation_queue = std::make_shared<VulkanQueue>(presentation_queue, queue_families.presentation);
        }
    }

    // Request compute queue
    if (requirements.compute) {
        if (queue_families.compute == queue_families.graphics) {
            m_compute_queue = m_graphics_queue;
        } else {
            VkQueue compute_queue;
            vkGetDeviceQueue(m_device, queue_families.compute, 0, &compute_queue);

            m_compute_queue = std::make_shared<VulkanQueue>(compute_queue, queue_families.compute);
        }
    }

    // Create command pools
    if (requirements.graphics) {
        m_graphics_command_pool = std::make_unique<VulkanCommandPool>(m_device, queue_families.graphics);
    }
    if (requirements.compute) {
        m_compute_command_pool = std::make_unique<VulkanCommandPool>(m_device, queue_families.compute);
    }
}

VulkanDevice::~VulkanDevice() {
    // Delete graphics_pool
    m_graphics_command_pool.reset();
    m_compute_command_pool.reset();

    vkDestroyDevice(m_device, nullptr);
}

VkCommandBuffer VulkanDevice::create_command_buffer(VulkanQueue::Type type) const {
    switch (type) {
    case VulkanQueue::Type::Graphics:
        PHOS_ASSERT(m_graphics_command_pool != nullptr, "Graphics queue was not requested");
        return m_graphics_command_pool->allocate(1)[0];
    case VulkanQueue::Type::Compute:
        PHOS_ASSERT(m_compute_command_pool != nullptr, "Compute queue was not requested");
        return m_compute_command_pool->allocate(1)[0];
    default:
        PHOS_FAIL("Not implemented");
    }
}

void VulkanDevice::free_command_buffer(VkCommandBuffer command_buffer, VulkanQueue::Type type) const {
    switch (type) {
    case VulkanQueue::Type::Graphics:
        PHOS_ASSERT(m_graphics_command_pool != nullptr, "Graphics queue was not requested");
        m_graphics_command_pool->free_command_buffer(command_buffer);
        break;
    case VulkanQueue::Type::Compute:
        PHOS_ASSERT(m_compute_command_pool != nullptr, "Compute queue was not requested");
        m_compute_command_pool->free_command_buffer(command_buffer);
        break;
    default:
        PHOS_FAIL("Not implemented");
    }
}

const std::shared_ptr<VulkanQueue>& VulkanDevice::get_queue_from_type(VulkanQueue::Type type) const {
    switch (type) {
    case VulkanQueue::Type::Graphics:
        PHOS_ASSERT(m_graphics_command_pool != nullptr, "Graphics queue was not requested");
        return m_graphics_queue;
    case VulkanQueue::Type::Compute:
        PHOS_ASSERT(m_compute_command_pool != nullptr, "Compute queue was not requested");
        return m_compute_queue;
    default:
        PHOS_FAIL("Not implemented");
    }
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
    constexpr bool graphics_compute_same_queue = false;

    uint32_t max_score = 0;
    std::optional<VulkanPhysicalDevice> max_device;

    for (const auto& device : physical_devices | std::views::filter(is_device_suitable)) {
        uint32_t score = 0;

        const auto queue_families = device.get_queue_families(reqs);

        if (graphics_transfer_same_queue && queue_families.graphics == queue_families.transfer)
            score += 10;

        if (graphics_presentation_same_queue && queue_families.graphics == queue_families.presentation)
            score += 10;

        if (graphics_compute_same_queue && queue_families.graphics == queue_families.compute)
            score += 10;

        if (score > max_score) {
            max_device = device;
            max_score = score;
        }
    }

    PHOS_ASSERT(max_device.has_value(), "There are no suitable devices");

    return max_device.value();
}

} // namespace Phos
