#include "vulkan_context.h"

#include <ranges>
#include <optional>

#include "core/window.h"

#include "renderer/vulkan_shader_module.h"
#include "renderer/vulkan_command_pool.h"

VulkanContext::VulkanContext(const std::shared_ptr<Window>& window) {
    m_instance = std::make_unique<VulkanInstance>(window);

    const auto physical_devices = m_instance->get_physical_devices();

    const std::vector<const char*> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    const auto selected_physical_device = select_physical_device(physical_devices, device_extensions);

    fmt::print("Selected physical device: {}\n", selected_physical_device.get_properties().deviceName);

    m_device = std::make_shared<VulkanDevice>(selected_physical_device, m_instance->get_surface(), device_extensions);
    m_swapchain = std::make_shared<VulkanSwapchain>(m_device, m_instance->get_surface(), window);

    const auto vertex = std::make_shared<VulkanShaderModule>(
        "../assets/shaders/vertex.spv", VulkanShaderModule::Stage::Vertex, m_device);
    const auto fragment = std::make_shared<VulkanShaderModule>(
        "../assets/shaders/fragment.spv", VulkanShaderModule::Stage::Fragment, m_device);

    m_render_pass = std::make_shared<VulkanRenderPass>(m_device);

    const auto pipeline_description = VulkanGraphicsPipeline::Description{
        .shader_modules = {vertex, fragment},
        .render_pass = m_render_pass,
    };
    m_pipeline = std::make_shared<VulkanGraphicsPipeline>(m_device, pipeline_description);

    // const auto graphics_queue_family = m_device->physical_device().get_queue_families({.graphics = true}).graphics;
    // VulkanCommandPool command_pool(m_device, graphics_queue_family, 2);

    // const auto command_buffers = command_pool.get_command_buffers();

    for (const auto& view : m_swapchain->get_image_views()) {
        const std::vector<VkImageView> attachments = {view};
        const auto framebuffer = std::make_shared<VulkanFramebuffer>(
            m_device, m_render_pass, window->get_width(), window->get_height(), attachments);

        m_present_framebuffers.push_back(framebuffer);
    }
}

VulkanPhysicalDevice VulkanContext::select_physical_device(
    const std::vector<VulkanPhysicalDevice>& physical_devices, const std::vector<const char*>& extensions) const {
    const VulkanPhysicalDevice::Requirements requirements = {
        .graphics = true,
        .transfer = true,
        .presentation = true,
        .surface = m_instance->get_surface(),

        .extensions = extensions,
    };

    const auto is_device_suitable = [&requirements](const VulkanPhysicalDevice& device) -> bool {
        return device.is_suitable(requirements);
    };

    // TODO: Should make these parameters configurable
    constexpr bool graphics_transfer_same_queue = true;
    constexpr bool graphics_presentation_same_queue = false;

    uint32_t max_score = 0;
    std::optional<VulkanPhysicalDevice> max_device;

    for (const auto& device : physical_devices | std::views::filter(is_device_suitable)) {
        uint32_t score = 0;

        const auto queue_families = device.get_queue_families(requirements);

        if (graphics_transfer_same_queue && queue_families.graphics == queue_families.transfer)
            score += 10;

        if (graphics_presentation_same_queue && queue_families.graphics == queue_families.presentation)
            score += 10;

        if (score > max_score) {
            max_device = device;
            max_score = score;
        }
    }

    CORE_ASSERT(max_device.has_value(), "There are no suitable devices")

    return max_device.value();
}
