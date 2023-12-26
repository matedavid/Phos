#include "vulkan_presenter.h"

#include "vk_core.h"

#include "core/window.h"
#include "scene/scene_renderer.h"
#include "managers/shader_manager.h"

#include "renderer/backend/renderer.h"
#include "renderer/backend/vulkan/vulkan_context.h"
#include "renderer/backend/vulkan/vulkan_device.h"
#include "renderer/backend/vulkan/vulkan_swapchain.h"
#include "renderer/backend/vulkan/vulkan_framebuffer.h"
#include "renderer/backend/vulkan/vulkan_graphics_pipeline.h"
#include "renderer/backend/vulkan/vulkan_render_pass.h"
#include "renderer/backend/vulkan/vulkan_command_buffer.h"

namespace Phos {

VulkanPresenter::VulkanPresenter(std::shared_ptr<ISceneRenderer> renderer, std::shared_ptr<Window> window)
      : m_renderer(std::move(renderer)), m_window(std::move(window)) {
    m_swapchain = std::make_shared<VulkanSwapchain>();

    m_graphics_queue = VulkanContext::device->get_graphics_queue();
    m_presentation_queue = VulkanContext::device->get_presentation_queue();

    const auto num_frames = Renderer::config().num_frames;
    for (uint32_t i = 0; i < num_frames; ++i) {
        m_command_buffers.push_back(std::make_shared<VulkanCommandBuffer>(VulkanQueue::Type::Graphics));
    }

    init();

    // Synchronization
    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    m_image_available_semaphores.resize(num_frames);
    m_rendering_finished_semaphores.resize(num_frames);
    m_wait_fences.resize(num_frames);

    for (uint32_t i = 0; i < num_frames; ++i) {
        VK_CHECK(vkCreateSemaphore(
            VulkanContext::device->handle(), &semaphore_create_info, nullptr, &m_image_available_semaphores[i]));
        VK_CHECK(vkCreateSemaphore(
            VulkanContext::device->handle(), &semaphore_create_info, nullptr, &m_rendering_finished_semaphores[i]));

        VK_CHECK(vkCreateFence(VulkanContext::device->handle(), &fence_create_info, nullptr, &m_wait_fences[i]));
    }
}

VulkanPresenter::~VulkanPresenter() {
    Renderer::wait_idle();

    for (uint32_t i = 0; i < Renderer::config().num_frames; ++i) {
        vkDestroySemaphore(VulkanContext::device->handle(), m_rendering_finished_semaphores[i], nullptr);
        vkDestroySemaphore(VulkanContext::device->handle(), m_image_available_semaphores[i], nullptr);
        vkDestroyFence(VulkanContext::device->handle(), m_wait_fences[i], nullptr);
    }
}

void VulkanPresenter::present() {
    VK_CHECK(vkWaitForFences(VulkanContext::device->handle(), 1, &m_wait_fences[m_current_frame], VK_TRUE, UINT64_MAX));
    VK_CHECK(vkResetFences(VulkanContext::device->handle(), 1, &m_wait_fences[m_current_frame]));

    m_swapchain->acquire_next_image(m_image_available_semaphores[m_current_frame], VK_NULL_HANDLE);

    const auto& native_cb = m_command_buffers[m_current_frame];
    const auto& command_buffer = std::dynamic_pointer_cast<CommandBuffer>(native_cb);

    const auto& pass = std::dynamic_pointer_cast<RenderPass>(m_presentation_pass);
    const auto& current_framebuffer = std::dynamic_pointer_cast<Framebuffer>(m_swapchain->get_current_framebuffer());

    command_buffer->record([&]() {
        pass->begin(command_buffer, current_framebuffer);

        Renderer::bind_graphics_pipeline(command_buffer, m_presentation_pipeline);
        Renderer::draw_screen_quad(command_buffer);

        pass->end(command_buffer);
    });

    const std::vector<VkPipelineStageFlags> wait_stages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    const std::vector<VkCommandBuffer> command_buffers = {native_cb->handle()};

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = (uint32_t)command_buffers.size();
    submit_info.pCommandBuffers = command_buffers.data();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &m_image_available_semaphores[m_current_frame];
    submit_info.pWaitDstStageMask = wait_stages.data();
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &m_rendering_finished_semaphores[m_current_frame];

    m_graphics_queue->submit(submit_info, m_wait_fences[m_current_frame]);

    // Present image
    const std::vector<VkSemaphore> wait_semaphores = {m_rendering_finished_semaphores[m_current_frame]};
    const auto result =
        m_presentation_queue->submitKHR(m_swapchain, m_swapchain->get_current_image_idx(), wait_semaphores);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        m_swapchain->recreate();
    } else if (result != VK_SUCCESS) {
        PHOS_FAIL("Failed to present image");
    }

    m_current_frame = (m_current_frame + 1) % Renderer::config().num_frames;
}

void VulkanPresenter::window_resized([[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height) {
    Renderer::wait_idle();
    vkQueueWaitIdle(m_presentation_queue->handle());

    init();
}

void VulkanPresenter::init() {
    // Presentation pass
    m_presentation_pipeline = std::make_shared<VulkanGraphicsPipeline>(GraphicsPipeline::Description{
        .shader = Renderer::shader_manager()->get_builtin_shader("Blending"),
        .target_framebuffer = std::dynamic_pointer_cast<Framebuffer>(m_swapchain->get_target_framebuffer()),
    });

    m_presentation_pipeline->add_input("uBlendingTexture", m_renderer->output_texture());

    m_presentation_pass = std::make_shared<VulkanRenderPass>(RenderPass::Description{
        .debug_name = "Presentation-Pass",
        .target_framebuffer = std::dynamic_pointer_cast<Framebuffer>(m_swapchain->get_target_framebuffer()),
    });
}

} // namespace Phos
