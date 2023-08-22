#include "viewport_panel.h"

#include <utility>

#include "../imgui/imgui_impl.h"

#include "scene/scene_renderer.h"

#include "renderer/backend/texture.h"
#include "renderer/backend/image.h"

ViewportPanel::ViewportPanel(std::string name, std::shared_ptr<Phos::ISceneRenderer> renderer)
      : m_name(std::move(name)), m_renderer(std::move(renderer)) {
    const auto& output_texture = m_renderer->output_texture();
    m_texture_id = ImGuiImpl::add_texture(output_texture);

    m_width = output_texture->get_image()->width();
    m_height = output_texture->get_image()->height();
}

void ViewportPanel::on_imgui_render() {
    ImGuiWindowClass window_class;
    window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
    ImGui::SetNextWindowClass(&window_class);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin(m_name.c_str(),
                 nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleVar();

    const auto viewport_node = ImGui::GetWindowDockNode();
    // const auto viewport_node = ImGui::DockBuilderGetCentralNode(dockspace_id);

    const auto width = static_cast<uint32_t>(viewport_node->Size.x);
    const auto height = static_cast<uint32_t>(viewport_node->Size.y);

    if (width != m_width || height != m_height) {
        m_width = width;
        m_height = height;

        m_renderer->window_resized(width, height);
        m_viewport_resized_callback(width, height);

        const auto& output_texture = m_renderer->output_texture();
        m_texture_id = ImGuiImpl::add_texture(output_texture);
    }

    m_renderer->render();
    ImGui::Image(m_texture_id, viewport_node->Size);

    ImGui::End();
}

void ViewportPanel::set_viewport_resized_callback(std::function<void(uint32_t, uint32_t)> func) {
    m_viewport_resized_callback = std::move(func);
}
