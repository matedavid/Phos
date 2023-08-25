#pragma once

#include "../imgui_impl.h"

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

class ImGuiVulkanImpl : public INativeImGuiImpl {
  public:
    explicit ImGuiVulkanImpl(std::shared_ptr<Phos::Window> window);
    ~ImGuiVulkanImpl() override;

    void new_frame() override;
    void render_frame(ImDrawData* draw_data) override;
    void present_frame() override;

    [[nodiscard]] ImTextureID add_texture(const std::shared_ptr<Phos::Texture>& texture) override;

  private:
    std::shared_ptr<Phos::Window> m_window;
    ImGui_ImplVulkanH_Window* m_wd;

    VkDescriptorPool m_descriptor_pool;
    bool m_rebuild_swapchain = false;

    uint32_t m_min_image_count = 2;

    void create_resize_window();

};
