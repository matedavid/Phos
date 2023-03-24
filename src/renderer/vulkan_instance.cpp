#include "vulkan_instance.h"

#include "renderer/vulkan_physical_device.h"
#include "core/window.h"

VulkanInstance::VulkanInstance(const std::shared_ptr<Window>& window) {
    const auto& required_extensions = window->get_vulkan_instance_extensions();

    VkApplicationInfo application_info{};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = "vulkan-renderer";
    application_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    application_info.pEngineName = "vulkan-renderer";
    application_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    application_info.apiVersion = VK_API_VERSION_1_3;

    const std::vector<const char*> validation_layers = {"VK_LAYER_KHRONOS_validation"};
    CORE_ASSERT(validation_layers_available(validation_layers), "Validation layers are not available")

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &application_info;
    create_info.enabledExtensionCount = (uint32_t)required_extensions.size();
    create_info.ppEnabledExtensionNames = required_extensions.data();
    create_info.enabledLayerCount = (uint32_t)validation_layers.size();
    create_info.ppEnabledLayerNames = validation_layers.data();

    VK_CHECK(vkCreateInstance(&create_info, nullptr, &m_instance))

    // Create Surface
    VK_CHECK(window->create_surface(m_instance, m_surface))
}

VulkanInstance::~VulkanInstance() {
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, nullptr);
}

std::vector<VulkanPhysicalDevice> VulkanInstance::get_physical_devices() const {
    uint32_t physical_device_count;
    VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &physical_device_count, nullptr))

    std::vector<VkPhysicalDevice> raw_physical_devices(physical_device_count);
    VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &physical_device_count, raw_physical_devices.data()))

    std::vector<VulkanPhysicalDevice> physical_devices;
    for (const auto& physical_device : raw_physical_devices) {
        physical_devices.emplace_back(physical_device);
    }

    return physical_devices;
}

bool VulkanInstance::validation_layers_available(const std::vector<const char*>& validation_layers) {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

    return std::all_of(validation_layers.begin(), validation_layers.end(), [&](const std::string_view& validation) {
        for (const auto& property : layers) {
            const std::string layer_name = property.layerName;
            if (validation == layer_name)
                return true;
        }
        return false;
    });
}
