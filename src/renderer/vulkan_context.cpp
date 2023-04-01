#include "vulkan_context.h"

#include <ranges>
#include <optional>

#include "core/window.h"

#include "renderer/vulkan_shader_module.h"

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

    const auto graphics_queue_family = m_device->physical_device().get_queue_families({.graphics = true}).graphics;
    m_command_pool = std::make_shared<VulkanCommandPool>(m_device, graphics_queue_family, MAX_FRAMES_IN_FLIGHT);
    m_command_buffer = m_command_pool->get_command_buffers()[0];

    vkGetDeviceQueue(m_device->handle(), graphics_queue_family, 0, &m_graphics_queue);

    for (const auto& view : m_swapchain->get_image_views()) {
        const std::vector<VkImageView> attachments = {view};
        const auto framebuffer = std::make_shared<VulkanFramebuffer>(
            m_device, m_render_pass, window->get_width(), window->get_height(), attachments);

        m_present_framebuffers.push_back(framebuffer);
    }

    // Synchronization
    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VK_CHECK(vkCreateSemaphore(m_device->handle(), &semaphoreCreateInfo, nullptr, &image_available_semaphore))
    VK_CHECK(vkCreateSemaphore(m_device->handle(), &semaphoreCreateInfo, nullptr, &render_finished_semaphore))
    VK_CHECK(vkCreateFence(m_device->handle(), &fenceCreateInfo, nullptr, &in_flight_fence))

    const std::vector<float> data = {0.0f, -0.5f, 0.0f, -0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 0.0f};
    m_vertex_buffer = std::make_unique<VulkanVertexBuffer>(m_device, data);
}

VulkanContext::~VulkanContext() {
    vkDeviceWaitIdle(m_device->handle());

    vkDestroySemaphore(m_device->handle(), image_available_semaphore, nullptr);
    vkDestroySemaphore(m_device->handle(), render_finished_semaphore, nullptr);
    vkDestroyFence(m_device->handle(), in_flight_fence, nullptr);
}

static uint32_t current_frame = 0;

void VulkanContext::update() {
    vkWaitForFences(m_device->handle(), 1, &in_flight_fence, VK_TRUE, UINT64_MAX);

    const auto next_image = m_swapchain->acquire_next_image(image_available_semaphore);

    vkResetFences(m_device->handle(), 1, &in_flight_fence);

    const auto& command_buffer = m_command_buffer;

    // Record command buffer
    command_buffer->begin();
    m_render_pass->begin(command_buffer, m_present_framebuffers[next_image]);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_present_framebuffers[0]->width();
    viewport.height = (float)m_present_framebuffers[0]->height();
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = {m_present_framebuffers[next_image]->width(), m_present_framebuffers[next_image]->height()};

    m_pipeline->bind(command_buffer);
    vkCmdSetViewport(command_buffer->handle(), 0, 1, &viewport);
    vkCmdSetScissor(command_buffer->handle(), 0, 1, &scissor);

    m_vertex_buffer->bind(command_buffer);
    vkCmdDraw(command_buffer->handle(), 3, 1, 0, 0);

    m_render_pass->end(command_buffer);
    command_buffer->end();
    // ===========================

    // Submit command buffer
    const std::array<VkPipelineStageFlags, 1> wait_stages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    const std::array<VkSemaphore, 1> wait_semaphores = {image_available_semaphore};
    const std::array<VkSemaphore, 1> signal_semaphores = {render_finished_semaphore};

    const std::array<VkCommandBuffer, 1> command_buffers = {command_buffer->handle()};

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = wait_semaphores.data();
    submitInfo.pWaitDstStageMask = wait_stages.data();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = command_buffers.data();
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signal_semaphores.data();

    if (vkQueueSubmit(m_graphics_queue, 1, &submitInfo, in_flight_fence) != VK_SUCCESS) {
        assert(false && "Failed to submit command buffer");
    }

    const std::array<VkSwapchainKHR, 1> swapchains = {m_swapchain->handle()};

    // Submit result to the swap chain
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &render_finished_semaphore;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains.data();
    present_info.pImageIndices = &next_image;

    const auto result = vkQueuePresentKHR(m_graphics_queue, &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        // recreateSwapchain();
    } else if (result != VK_SUCCESS) {
        CORE_FAIL("Failed to present image");
    }

    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
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
