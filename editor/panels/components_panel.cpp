#include "components_panel.h"

#include "scene/scene.h"

#include "renderer/backend/material.h"

ComponentsPanel::ComponentsPanel(std::string name, std::shared_ptr<Phos::Scene> scene)
      : m_name(std::move(name)), m_scene(std::move(scene)) {}

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

template <typename T>
void render_component(T& component);

template <typename T>
void render_component(const Phos::Entity& entity) {
    if (!entity.has_component<T>())
        return;

    render_component<T>(entity.get_component<T>());
    ImGui::Separator();
}

void ComponentsPanel::on_imgui_render() {
    ImGui::Begin(m_name.c_str());

    if (m_selected_entity.has_value()) {
        const auto& entity = *m_selected_entity;

        render_component<Phos::NameComponent>(entity);

        render_component<Phos::TransformComponent>(entity);

        render_component<Phos::MeshRendererComponent>(entity);
    }

    ImGui::End();
}

//
// Specific render_component
//

template <>
void render_component<Phos::NameComponent>(Phos::NameComponent& component) {
    const auto& entity_name = component.name;
    ImGui::Text("Name: %s", entity_name.c_str());
}

template <>
void render_component<Phos::TransformComponent>(Phos::TransformComponent& component) {
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Transform");

    if (!ImGui::BeginTable("Transform", 4))
        return;

    // Position
    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Position:");

    ImGui::TableSetColumnIndex(1);
    render_label_input("X", "Position", &component.position.x);

    ImGui::TableSetColumnIndex(2);
    render_label_input("Y", "Position", &component.position.y);

    ImGui::TableSetColumnIndex(3);
    render_label_input("Z", "Position", &component.position.z);

    // Rotation
    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Rotation:");

    ImGui::TableSetColumnIndex(1);
    render_label_input("X", "Rotation", &component.rotation.x);

    ImGui::TableSetColumnIndex(2);
    render_label_input("Y", "Rotation", &component.rotation.y);

    ImGui::TableSetColumnIndex(3);
    render_label_input("Z", "Rotation", &component.rotation.z);

    // Scale
    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Scale:");

    ImGui::TableSetColumnIndex(1);
    render_label_input("X", "Scale", &component.scale.x);

    ImGui::TableSetColumnIndex(2);
    render_label_input("Y", "Scale", &component.scale.y);

    ImGui::TableSetColumnIndex(3);
    render_label_input("Z", "Scale", &component.scale.z);

    ImGui::EndTable();
}

template <>
void render_component<Phos::MeshRendererComponent>(Phos::MeshRendererComponent& component) {
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
    ImGui::Text("%s", component.material->name().c_str());

    ImGui::EndTable();
}
