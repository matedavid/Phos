#include "components_panel.h"

#include "scene/scene.h"

#include "renderer/backend/material.h"

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
        auto& transform = m_selected_entity->get_component<Phos::TransformComponent>();
        render_transform_component(transform);

        ImGui::Separator();

        const auto component_names = m_selected_entity->get_component_names();
        for (const auto& name : component_names) {
            bool rendered = render_component(name, *m_selected_entity);
            if (rendered)
                ImGui::Separator();
        }
    }

    ImGui::End();
}

void ComponentsPanel::set_selected_entity(const Phos::Entity& entity) {
    m_selected_entity = entity;
}

template <typename T>
void render_label_input(const std::string& label, const std::string& group, T* value) {
    ImGui::AlignTextToFramePadding();

    ImGui::TextUnformatted(label.c_str());
    ImGui::SameLine();

    const std::string input_label = "##" + group + label;

    if (typeid(T) == typeid(float)) {
        ImGui::InputFloat(input_label.c_str(), value, 0.0f, 0.0f, "%.1f");
    } else {
        PS_FAIL("Unsupported input type")
    }
}

bool ComponentsPanel::render_component(const std::string& name, const Phos::Entity& entity) const {
    if (name == typeid(Phos::MeshRendererComponent).name()) {
        auto& mesh_renderer = entity.get_component<Phos::MeshRendererComponent>();
        render_mesh_renderer_component(mesh_renderer);
        return true;
    }

    return false;
}

void ComponentsPanel::render_transform_component(Phos::TransformComponent& transform) const {
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Transform");

    if (!ImGui::BeginTable("Transform", 4))
        return;

    // Position
    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Position:");

    ImGui::TableSetColumnIndex(1);
    render_label_input("X", "Position", &transform.position.x);

    ImGui::TableSetColumnIndex(2);
    render_label_input("Y", "Position", &transform.position.y);

    ImGui::TableSetColumnIndex(3);
    render_label_input("Z", "Position", &transform.position.z);

    // Rotation
    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Rotation:");

    ImGui::TableSetColumnIndex(1);
    render_label_input("X", "Rotation", &transform.rotation.x);

    ImGui::TableSetColumnIndex(2);
    render_label_input("Y", "Rotation", &transform.rotation.y);

    ImGui::TableSetColumnIndex(3);
    render_label_input("Z", "Rotation", &transform.rotation.z);

    // Scale
    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Scale:");

    ImGui::TableSetColumnIndex(1);
    render_label_input("X", "Scale", &transform.scale.x);

    ImGui::TableSetColumnIndex(2);
    render_label_input("Y", "Scale", &transform.scale.y);

    ImGui::TableSetColumnIndex(3);
    render_label_input("Z", "Scale", &transform.scale.z);

    ImGui::EndTable();
}

void ComponentsPanel::render_mesh_renderer_component(Phos::MeshRendererComponent& mesh_renderer) const {
    ImGui::AlignTextToFramePadding();
    ImGui::Text("MeshRenderer");

    if (!ImGui::BeginTable("MeshRenderer", 2))
        return;

    ImGui::TableNextRow();

    // Mesh
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Mesh:");

    ImGui::TableSetColumnIndex(1);
    ImGui::Text("-");

    ImGui::TableNextRow();

    // Material
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Material:");

    ImGui::TableSetColumnIndex(1);
    ImGui::Text("%s", mesh_renderer.material->name().c_str());

    ImGui::EndTable();
}
