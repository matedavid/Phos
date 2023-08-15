#include "core/entry_point.h"
#include "core/layer.h"
#include "core/application.h"
#include "core/window.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "renderer/backend/renderer.h"
#include "renderer/backend/vulkan/vulkan_context.h"
#include "renderer/backend/vulkan/vulkan_instance.h"
#include "renderer/backend/vulkan/vulkan_device.h"
#include "renderer/backend/vulkan/vulkan_command_buffer.h"

constexpr uint32_t WIDTH = 1280;
constexpr uint32_t HEIGHT = 960;

class EditorLayer : public Phos::Layer {
  public:
    EditorLayer() {
        // Descriptor pool
        VkDescriptorPoolSize pool_sizes[] = {
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1;
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        auto err =
            vkCreateDescriptorPool(Phos::VulkanContext::device->handle(), &pool_info, nullptr, &m_descriptor_pool);
        VK_CHECK(err);

        setup_vulkan_window();

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForVulkan(Phos::Application::instance()->get_window()->handle(), true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = Phos::VulkanContext::instance->handle();
        init_info.PhysicalDevice = Phos::VulkanContext::device->physical_device().handle();
        init_info.Device = Phos::VulkanContext::device->handle();
        init_info.QueueFamily = Phos::VulkanContext::device->get_graphics_queue()->family();
        init_info.Queue = Phos::VulkanContext::device->get_graphics_queue()->handle();
        // init_info.PipelineCache = g_PipelineCache;
        init_info.DescriptorPool = m_descriptor_pool;
        init_info.Subpass = 0;
        init_info.MinImageCount = 2;
        init_info.ImageCount = wd->ImageCount;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = nullptr;
        init_info.CheckVkResultFn = [](VkResult result) {
            VK_CHECK(result);
        };
        ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);

        // Upload fonts
        Phos::VulkanCommandBuffer::submit_single_time(Phos::VulkanQueue::Type::Graphics,
                                                      [](const std::shared_ptr<Phos::VulkanCommandBuffer>& cb) {
                                                          ImGui_ImplVulkan_CreateFontsTexture(cb->handle());
                                                      });

        Phos::Renderer::wait_idle();
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    ~EditorLayer() override {
        Phos::Renderer::wait_idle();

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        vkDestroyDescriptorPool(Phos::VulkanContext::device->handle(), m_descriptor_pool, nullptr);

        ImGui_ImplVulkanH_DestroyWindow(
            Phos::VulkanContext::instance->handle(), Phos::VulkanContext::device->handle(), wd, nullptr);
    }

    void on_update([[maybe_unused]] double ts) override {
        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        bool show = true;
        ImGui::ShowDemoWindow(&show);

        // Rendering
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        if (!is_minimized) {
            wd->ClearValue.color.float32[0] = 0.0f;
            wd->ClearValue.color.float32[1] = 0.0f;
            wd->ClearValue.color.float32[2] = 0.0f;
            wd->ClearValue.color.float32[3] = 1.0f;
            FrameRender(wd, draw_data);
            FramePresent(wd);
        }
    }

  private:
    ImGui_ImplVulkanH_Window* wd = new ImGui_ImplVulkanH_Window{};
    VkDescriptorPool m_descriptor_pool;

    void setup_vulkan_window() {
        Phos::Application::instance()->get_window()->create_surface(Phos::VulkanContext::instance->handle(),
                                                                    wd->Surface);

        // wd->Surface = Phos::VulkanContext::instance->get_surface();

        // Select Surface Format
        const VkFormat requestSurfaceImageFormat[] = {
            VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM};
        const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        wd->SurfaceFormat =
            ImGui_ImplVulkanH_SelectSurfaceFormat(Phos::VulkanContext::device->physical_device().handle(),
                                                  wd->Surface,
                                                  requestSurfaceImageFormat,
                                                  (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat),
                                                  requestSurfaceColorSpace);
        // Select Present Mode
        VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_FIFO_KHR};
        wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(Phos::VulkanContext::device->physical_device().handle(),
                                                              wd->Surface,
                                                              &present_modes[0],
                                                              IM_ARRAYSIZE(present_modes));

        const auto graphics_queue = Phos::VulkanContext::device->get_graphics_queue();
        ImGui_ImplVulkanH_CreateOrResizeWindow(Phos::VulkanContext::instance->handle(),
                                               Phos::VulkanContext::device->physical_device().handle(),
                                               Phos::VulkanContext::device->handle(),
                                               wd,
                                               graphics_queue->family(),
                                               nullptr,
                                               WIDTH,
                                               HEIGHT,
                                               2);
    }

    static void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data) {
        VkResult err;

        VkDevice device = Phos::VulkanContext::device->handle();

        VkSemaphore image_acquired_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
        VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
        err = vkAcquireNextImageKHR(
            device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
        if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
            // g_SwapChainRebuild = true;
            return;
        }
        VK_CHECK(err);

        ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
        {
            err = vkWaitForFences(
                device, 1, &fd->Fence, VK_TRUE, UINT64_MAX); // wait indefinitely instead of periodically checking
            VK_CHECK(err);

            err = vkResetFences(device, 1, &fd->Fence);
            VK_CHECK(err);
        }
        {
            err = vkResetCommandPool(device, fd->CommandPool, 0);
            VK_CHECK(err);
            VkCommandBufferBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
            VK_CHECK(err);
        }
        {
            VkRenderPassBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            info.renderPass = wd->RenderPass;
            info.framebuffer = fd->Framebuffer;
            info.renderArea.extent.width = static_cast<uint32_t>(wd->Width);
            info.renderArea.extent.height = static_cast<uint32_t>(wd->Height);
            info.clearValueCount = 1;
            info.pClearValues = &wd->ClearValue;
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

    static void FramePresent(ImGui_ImplVulkanH_Window* wd) {
        VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
        VkPresentInfoKHR info = {};
        info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &render_complete_semaphore;
        info.swapchainCount = 1;
        info.pSwapchains = &wd->Swapchain;
        info.pImageIndices = &wd->FrameIndex;
        VkResult err = vkQueuePresentKHR(Phos::VulkanContext::device->get_graphics_queue()->handle(), &info);
        if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
            // g_SwapChainRebuild = true;
            return;
        }
        VK_CHECK(err);
        wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->ImageCount; // Now we can use the next set of semaphores
    }
};

Phos::Application* create_application() {
    auto* application = new Phos::Application("Phos Editor", WIDTH, HEIGHT);
    application->push_layer<EditorLayer>();

    return application;
}
