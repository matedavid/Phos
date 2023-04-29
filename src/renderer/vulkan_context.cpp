#include "vulkan_context.h"

#include "core/window.h"
#include "renderer/vulkan_instance.h"
#include "renderer/vulkan_device.h"
#include "renderer/vulkan_descriptors.h"

namespace Phos {

std::unique_ptr<VulkanInstance> VulkanContext::instance = nullptr;
std::unique_ptr<VulkanDevice> VulkanContext::device = nullptr;
std::shared_ptr<VulkanDescriptorLayoutCache> VulkanContext::descriptor_layout_cache = nullptr;

void VulkanContext::init(const std::shared_ptr<Window>& window) {
    instance = std::make_unique<VulkanInstance>(window);

    const std::vector<std::string_view> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    const auto device_requirements = VulkanPhysicalDevice::Requirements{
        .graphics = true,
        .transfer = true,
        .presentation = true,
        .surface = instance->get_surface(),

        .extensions = device_extensions,
    };
    device = std::make_unique<VulkanDevice>(instance, device_requirements);

    descriptor_layout_cache = std::make_shared<VulkanDescriptorLayoutCache>();
}

void VulkanContext::free() {
    descriptor_layout_cache.reset();
    device.reset();
    instance.reset();
}

} // namespace Phos