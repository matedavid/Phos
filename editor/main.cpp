#include "core/entry_point.h"
#include "core/layer.h"
#include "core/application.h"
#include "core/window.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "asset/asset_manager.h"

#include "scene/scene_renderer.h"
#include "scene/scene.h"
#include "scene/entity.h"

#include "managers/shader_manager.h"

#include "renderer/camera.h"
#include "renderer/mesh.h"
#include "renderer/deferred_renderer.h"
#include "renderer/backend/renderer.h"
#include "renderer/backend/material.h"
#include "renderer/backend/shader.h"
#include "renderer/backend/vulkan/vulkan_descriptors.h"
#include "renderer/backend/vulkan/vulkan_context.h"
#include "renderer/backend/vulkan/vulkan_instance.h"
#include "renderer/backend/vulkan/vulkan_device.h"
#include "renderer/backend/vulkan/vulkan_command_buffer.h"
#include "renderer/backend/vulkan/vulkan_texture.h"
#include "renderer/backend/vulkan/vulkan_image.h"

constexpr uint32_t WIDTH = 1280;
constexpr uint32_t HEIGHT = 960;

class EditorLayer : public Phos::Layer {
  public:
    EditorLayer() {
        m_asset_manager =
            std::make_shared<Phos::AssetManager>(std::make_shared<Phos::AssetPack>("../assets/asset_pack.psap"));
        m_scene = std::make_shared<Phos::Scene>("Example");
        m_renderer = std::make_shared<Phos::DeferredRenderer>(m_scene);

        create_scene();

        const auto aspect_ratio = WIDTH / HEIGHT;
        const auto m_camera =
            std::make_shared<Phos::PerspectiveCamera>(glm::radians(90.0f), aspect_ratio, 0.001f, 40.0f);
        m_camera->set_position({0.0f, 3.0f, 7.0f});
        m_camera->rotate(glm::vec2(0.0f, glm::radians(30.0f)));

        m_scene->set_camera(m_camera);

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
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking

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
            VK_CHECK(result)
        };
        ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);

        // Upload fonts
        Phos::VulkanCommandBuffer::submit_single_time(Phos::VulkanQueue::Type::Graphics,
                                                      [](const std::shared_ptr<Phos::VulkanCommandBuffer>& cb) {
                                                          ImGui_ImplVulkan_CreateFontsTexture(cb->handle());
                                                      });

        Phos::Renderer::wait_idle();
        ImGui_ImplVulkan_DestroyFontUploadObjects();

        // Image Descriptors
        const auto output_texture = std::dynamic_pointer_cast<Phos::VulkanTexture>(m_renderer->output_texture());
        const auto image = std::dynamic_pointer_cast<Phos::VulkanImage>(output_texture->get_image());

        m_set = ImGui_ImplVulkan_AddTexture(
            output_texture->sampler(), image->view(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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

        const auto viewport = ImGui::GetMainViewport();

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        window_flags |= ImGuiWindowFlags_NoBackground;

        static ImGuiID dockspace_id;
        static bool first_time = true;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace", nullptr, window_flags);
        ImGui::PopStyleVar();
        ImGui::PopStyleVar(2);

        dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

        if (first_time) {
            first_time = false;

            ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
            ImGui::DockBuilderAddNode(
                dockspace_id, ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace); // Add empty node
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

            auto dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, nullptr, &dockspace_id);
            auto dock_id_down = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.25f, nullptr, &dockspace_id);

            // we now dock our windows into the docking node we made above
            ImGui::DockBuilderDockWindow("Down", dock_id_down);
            ImGui::DockBuilderDockWindow("Left", dock_id_left);
            ImGui::DockBuilderDockWindow("Viewport", dockspace_id);
            ImGui::DockBuilderFinish(dockspace_id);
        }

        ImGui::End();

        ImGui::Begin("Left");
        ImGui::Text("Hello, left!");
        ImGui::End();

        ImGui::Begin("Down");
        ImGui::Text("Hello, down!");
        ImGui::End();

        ImGui::Begin("Viewport");
        m_renderer->render();
        ImGui::Image((ImTextureID)m_set, ImVec2(WIDTH, HEIGHT));
        ImGui::End();

        // Rendering
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        if (!is_minimized) {
            wd->ClearValue.color.float32[0] = 0.0f;
            wd->ClearValue.color.float32[1] = 0.0f;
            wd->ClearValue.color.float32[2] = 0.0f;
            wd->ClearValue.color.float32[3] = 1.0f;
            render_frame(wd, draw_data);
            present_frame(wd);
        }
    }

  private:
    std::shared_ptr<Phos::ISceneRenderer> m_renderer;
    std::shared_ptr<Phos::Scene> m_scene;
    std::shared_ptr<Phos::AssetManager> m_asset_manager;

    ImGui_ImplVulkanH_Window* wd = new ImGui_ImplVulkanH_Window{};
    VkDescriptorPool m_descriptor_pool;
    VkDescriptorSet m_set;

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

    static void render_frame(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data) {
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

    static void present_frame(ImGui_ImplVulkanH_Window* wd) {
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

    void create_scene() {
        const auto sphere_mesh = m_asset_manager->load<Phos::Mesh>("../assets/models/sphere/sphere.psa");
        const auto cube_mesh = m_asset_manager->load<Phos::Mesh>("../assets/models/cube/cube.psa");

        // Floor
        const auto floor_material = Phos::Material::create(
            Phos::Renderer::shader_manager()->get_builtin_shader("PBR.Geometry.Deferred"), "Floor Material");

        floor_material->set("uAlbedoMap", Phos::Texture::create("../assets/wood_mat/hardwood-brown-planks-albedo.png"));
        floor_material->set("uMetallicMap",
                            Phos::Texture::create("../assets/wood_mat/hardwood-brown-planks-metallic.png"));
        floor_material->set("uRoughnessMap",
                            Phos::Texture::create("../assets/wood_mat/hardwood-brown-planks-roughness.png"));
        floor_material->set("uAOMap", Phos::Texture::create("../assets/wood_mat/hardwood-brown-planks-ao.png"));
        floor_material->set("uNormalMap",
                            Phos::Texture::create("../assets/wood_mat/hardwood-brown-planks-normal-ogl.png"));

        PS_ASSERT(floor_material->bake(), "Failed to bake floor material")

        const auto light_material = Phos::Material::create(
            Phos::Renderer::shader_manager()->get_builtin_shader("PBR.Geometry.Deferred"), "Light Material");
        PS_ASSERT(light_material->bake(), "Failed to bake light material")

        auto floor_entity = m_scene->create_entity();
        floor_entity.get_component<Phos::TransformComponent>().scale = glm::vec3(10.0f, 0.25f, 10.0f);
        floor_entity.get_component<Phos::TransformComponent>().position = glm::vec3(0.0f, -0.25f, 0.0f);
        floor_entity.add_component<Phos::MeshRendererComponent>({
            .mesh = cube_mesh,
            .material = floor_material,
        });

        auto wall_entity = m_scene->create_entity();
        wall_entity.get_component<Phos::TransformComponent>().scale = glm::vec3(10.0f, 0.25f, 7.0f);
        wall_entity.get_component<Phos::TransformComponent>().position = glm::vec3(0.0f, 6.5f, -10.0f);
        wall_entity.get_component<Phos::TransformComponent>().rotation = glm::vec3(glm::radians(90.0f), 0.0f, 0.0f);
        wall_entity.add_component<Phos::MeshRendererComponent>({
            .mesh = cube_mesh,
            .material = floor_material,
        });

        // Spheres
        const uint32_t number = 4;
        for (uint32_t metallic_idx = 0; metallic_idx < number; ++metallic_idx) {
            for (uint32_t roughness_idx = 0; roughness_idx < number; ++roughness_idx) {
                const auto sphere_material = Phos::Material::create(
                    Phos::Renderer::shader_manager()->get_builtin_shader("PBR.Geometry.Deferred"), "Sphere Material");

                const float metallic = (float)(metallic_idx + 1) / (float)number;
                const float roughness = (float)(roughness_idx + 1) / (float)number;

                sphere_material->set("uMaterialInfo.albedo", glm::vec3(0.8f, 0.05f, 0.05f));
                sphere_material->set("uMaterialInfo.metallic", metallic);
                sphere_material->set("uMaterialInfo.roughness", roughness);
                sphere_material->set("uMaterialInfo.ao", 1.0f);

                PS_ASSERT(sphere_material->bake(), "Failed to bake sphere material {} {}", metallic_idx, roughness_idx)

                auto sphere_entity = m_scene->create_entity();
                sphere_entity.get_component<Phos::TransformComponent>().position =
                    glm::vec3(metallic_idx * 2, roughness_idx * 2 + 1, -2.0f);
                sphere_entity.get_component<Phos::TransformComponent>().scale = glm::vec3(0.6f);
                sphere_entity.get_component<Phos::TransformComponent>().rotation =
                    glm::vec3(glm::radians(0.0f), 0.0f, 0.0f);
                sphere_entity.add_component<Phos::MeshRendererComponent>({
                    .mesh = sphere_mesh,
                    .material = sphere_material,
                });
            }
        }

        // Lights
        const std::vector<glm::vec3> light_positions = {
            glm::vec3(2.0f, 3.0f, 1.5f),
            glm::vec3(4.0f, 2.0f, 1.5f),
            glm::vec3(5.0f, 6.0f, 1.5f),
            glm::vec3(5.0f, 1.0f, 1.5f),
        };

        for (const auto& light_pos : light_positions) {
            auto light_entity = m_scene->create_entity();
            light_entity.get_component<Phos::TransformComponent>().position = light_pos;
            light_entity.get_component<Phos::TransformComponent>().scale = glm::vec3(0.15f);

            light_entity.add_component<Phos::LightComponent>({
                .light_type = Phos::Light::Type::Point,
                .color = glm::vec4(1.0f),
            });

            light_entity.add_component<Phos::MeshRendererComponent>({
                .mesh = cube_mesh,
                .material = light_material,
            });
        }

        auto directional_light_entity = m_scene->create_entity();
        directional_light_entity.get_component<Phos::TransformComponent>().position = glm::vec3(2.0f, 17.0f, 9.0f);
        directional_light_entity.get_component<Phos::TransformComponent>().rotation =
            glm::vec3(glm::radians(40.0f), glm::radians(180.0f), glm::radians(0.0f));
        directional_light_entity.get_component<Phos::TransformComponent>().scale = glm::vec3(0.15f);

        directional_light_entity.add_component<Phos::MeshRendererComponent>({
            .mesh = cube_mesh,
            .material = light_material,
        });

        directional_light_entity.add_component<Phos::LightComponent>({
            .light_type = Phos::Light::Type::Directional,
            .color = glm::vec4(1.0f),

            .shadow_type = Phos::Light::ShadowType::Hard,
        });
    }
};

Phos::Application* create_application() {
    auto* application = new Phos::Application("Phos Editor", WIDTH, HEIGHT);
    application->push_layer<EditorLayer>();

    return application;
}
