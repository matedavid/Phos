#include <imgui.h>
#include <imgui_internal.h>

#include "utility/logging.h"

#include "core/window.h"
#include "renderer/backend/renderer.h"

#include "vulkan/imgui_vulkan_impl.h"
#include "imgui_impl.h"

std::shared_ptr<INativeImGuiImpl> ImGuiImpl::m_native_impl = nullptr;

void ImGuiImpl::initialize(const std::shared_ptr<Phos::Window>& window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    switch (Phos::Renderer::graphics_api()) {
    case Phos::GraphicsAPI::Vulkan:
        m_native_impl = std::make_shared<ImGuiVulkanImpl>(window);
        break;
    default:
        PHOS_FAIL("Vulkan is the only supported api");
    }
}

void ImGuiImpl::shutdown() {
    m_native_impl.reset();
    ImGui::DestroyContext();
}

void ImGuiImpl::new_frame() {
    m_native_impl->new_frame();
}

void ImGuiImpl::render_frame(ImDrawData* draw_data) {
    m_native_impl->render_frame(draw_data);
}

void ImGuiImpl::present_frame() {
    m_native_impl->present_frame();
}

ImTextureID ImGuiImpl::add_texture(const std::shared_ptr<Phos::Texture>& texture) {
    return m_native_impl->add_texture(texture);
}
