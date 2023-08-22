#include "components_panel.h"

#include "scene/scene.h"

ComponentsPanel::ComponentsPanel(std::string name, std::shared_ptr<Phos::Scene> scene)
      : m_name(std::move(name)), m_scene(std::move(scene)) {}

void ComponentsPanel::on_imgui_render() {
    ImGui::Begin(m_name.c_str());

    if (m_selected_entity.has_value()) {
        // Name component
        const auto& entity_name = m_selected_entity->get_component<Phos::NameComponent>().name;
        ImGui::Text("Name: %s", entity_name.c_str());

        ImGui::Separator();

        // Transform component
        const auto& transform = m_selected_entity->get_component<Phos::TransformComponent>();
        ImGui::Text("Position: %f %f %f", transform.position.x, transform.position.y, transform.position.z);
        ImGui::Text("Rotation: %f %f %f", transform.rotation.x, transform.rotation.y, transform.rotation.z);
        ImGui::Text("Scale: %f %f %f", transform.scale.x, transform.scale.y, transform.scale.z);

        ImGui::Separator();

        // const auto& component_names = m_selected_entity.value().get_component_names();
    }

    ImGui::End();
}

void ComponentsPanel::set_selected_entity(const Phos::Entity& entity) {
    m_selected_entity = entity;
}
