#include "imgui_vulkan_impl.h"

#include <utility>

#include "renderer/backend/vulkan/vk_core.h"

#include "core/window.h"
#include "core/application.h"

#include "renderer/backend/renderer.h"
#include "renderer/backend/vulkan/vulkan_context.h"
#include "renderer/backend/vulkan/vulkan_instance.h"
#include "renderer/backend/vulkan/vulkan_device.h"
#include "renderer/backend/vulkan/vulkan_command_buffer.h"
#include "renderer/backend/vulkan/vulkan_texture.h"
#include "renderer/backend/vulkan/vulkan_image.h"

ImGuiVulkanImpl::ImGuiVulkanImpl(std::shared_ptr<Phos::Window> window) : m_window(std::move(window)) {
    m_wd = new ImGui_ImplVulkanH_Window();
    m_window->create_surface(Phos::VulkanContext::instance->handle(), m_wd->Surface);

    // Select Surface Format
    const VkFormat request_surface_formats[] = {
        VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM};
    const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    m_wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(Phos::VulkanContext::device->physical_device().handle(),
                                                                m_wd->Surface,
                                                                request_surface_formats,
                                                                (size_t)IM_ARRAYSIZE(request_surface_formats),
                                                                requestSurfaceColorSpace);
    // Select Present Mode
    VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_FIFO_KHR};
    m_wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(Phos::VulkanContext::device->physical_device().handle(),
                                                            m_wd->Surface,
                                                            &present_modes[0],
                                                            IM_ARRAYSIZE(present_modes));

    create_resize_window();

    // Descriptor pool
    VkDescriptorPoolSize pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 100},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100},
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    VK_CHECK(vkCreateDescriptorPool(Phos::VulkanContext::device->handle(), &pool_info, nullptr, &m_descriptor_pool));

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(Phos::Application::instance()->get_window()->handle(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = Phos::VulkanContext::instance->handle();
    init_info.PhysicalDevice = Phos::VulkanContext::device->physical_device().handle();
    init_info.Device = Phos::VulkanContext::device->handle();
    init_info.QueueFamily = Phos::VulkanContext::device->get_graphics_queue()->family();
    init_info.Queue = Phos::VulkanContext::device->get_graphics_queue()->handle();
    init_info.DescriptorPool = m_descriptor_pool;
    init_info.Subpass = 0;
    init_info.MinImageCount = 2;
    init_info.ImageCount = m_wd->ImageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = [](VkResult result) {
        VK_CHECK(result);
    };
    ImGui_ImplVulkan_Init(&init_info, m_wd->RenderPass);

    // Upload fonts
    ImGui_ImplVulkan_CreateFontsTexture();
}

ImGuiVulkanImpl::~ImGuiVulkanImpl() {
    Phos::Renderer::wait_idle();

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    vkDestroyDescriptorPool(Phos::VulkanContext::device->handle(), m_descriptor_pool, nullptr);

    ImGui_ImplVulkanH_DestroyWindow(
        Phos::VulkanContext::instance->handle(), Phos::VulkanContext::device->handle(), m_wd, nullptr);
}

void ImGuiVulkanImpl::new_frame() {
    if (m_rebuild_swapchain) {
        const auto width = m_window->get_width();
        const auto height = m_window->get_width();

        if (width > 0 && height > 0) {
            ImGui_ImplVulkan_SetMinImageCount(m_min_image_count);
            create_resize_window();

            m_wd->FrameIndex = 0;
            m_rebuild_swapchain = false;
        }
    }

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
}

void ImGuiVulkanImpl::render_frame(ImDrawData* draw_data) {
    VkDevice device = Phos::VulkanContext::device->handle();

    VkSemaphore image_acquired_semaphore = m_wd->FrameSemaphores[m_wd->SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore = m_wd->FrameSemaphores[m_wd->SemaphoreIndex].RenderCompleteSemaphore;
    auto err = vkAcquireNextImageKHR(
        device, m_wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &m_wd->FrameIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
        m_rebuild_swapchain = true;
        return;
    }
    VK_CHECK(err);

    ImGui_ImplVulkanH_Frame* fd = &m_wd->Frames[m_wd->FrameIndex];
    {
        VK_CHECK(vkWaitForFences(
            device, 1, &fd->Fence, VK_TRUE, UINT64_MAX)); // wait indefinitely instead of periodically checking
        VK_CHECK(vkResetFences(device, 1, &fd->Fence));
    }

    {
        VK_CHECK(vkResetCommandPool(device, fd->CommandPool, 0));
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VK_CHECK(vkBeginCommandBuffer(fd->CommandBuffer, &info));
    }

    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = m_wd->RenderPass;
        info.framebuffer = fd->Framebuffer;
        info.renderArea.extent.width = static_cast<uint32_t>(m_wd->Width);
        info.renderArea.extent.height = static_cast<uint32_t>(m_wd->Height);
        info.clearValueCount = 1;
        info.pClearValues = &m_wd->ClearValue;
        vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

    // Submit command buffer
    vkCmdEndRenderPass(fd->CommandBuffer);
    {
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &image_acquired_semaphore;
        info.pWaitDstStageMask = &wait_stage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &fd->CommandBuffer;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &render_complete_semaphore;

        err = vkEndCommandBuffer(fd->CommandBuffer);
        VK_CHECK(err);
        err = vkQueueSubmit(Phos::VulkanContext::device->get_graphics_queue()->handle(), 1, &info, fd->Fence);
        VK_CHECK(err);
    }
}

void ImGuiVulkanImpl::present_frame() {
    if (m_rebuild_swapchain)
        return;

    VkSemaphore render_complete_semaphore = m_wd->FrameSemaphores[m_wd->SemaphoreIndex].RenderCompleteSemaphore;
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &m_wd->Swapchain;
    info.pImageIndices = &m_wd->FrameIndex;

    auto err = vkQueuePresentKHR(Phos::VulkanContext::device->get_graphics_queue()->handle(), &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
        m_rebuild_swapchain = true;
        return;
    }
    VK_CHECK(err);
    m_wd->SemaphoreIndex = (m_wd->SemaphoreIndex + 1) % m_wd->ImageCount; // Now we can use the next set of semaphores
}

ImTextureID ImGuiVulkanImpl::add_texture(const std::shared_ptr<Phos::Texture>& texture) {
    const auto& native_texture = std::dynamic_pointer_cast<Phos::VulkanTexture>(texture);
    const auto& native_image = std::dynamic_pointer_cast<Phos::VulkanImage>(texture->get_image());

    return ImGui_ImplVulkan_AddTexture(
        native_texture->sampler(), native_image->view(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void ImGuiVulkanImpl::create_resize_window() {
    const auto graphics_queue = Phos::VulkanContext::device->get_graphics_queue();
    ImGui_ImplVulkanH_CreateOrResizeWindow(Phos::VulkanContext::instance->handle(),
                                           Phos::VulkanContext::device->physical_device().handle(),
                                           Phos::VulkanContext::device->handle(),
                                           m_wd,
                                           graphics_queue->family(),
                                           nullptr,
                                           static_cast<int32_t>(m_window->get_width()),
                                           static_cast<int32_t>(m_window->get_height()),
                                           m_min_image_count);
}
