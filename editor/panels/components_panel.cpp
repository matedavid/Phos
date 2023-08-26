#include "components_panel.h"

#include <misc/cpp/imgui_stdlib.h>

#include "scene/scene.h"
#include "renderer/backend/material.h"

ComponentsPanel::ComponentsPanel(std::string name, std::shared_ptr<Phos::Scene> scene)
      : m_name(std::move(name)), m_scene(std::move(scene)) {}

void ComponentsPanel::set_selected_entity(const Phos::Entity& entity) {
    m_selected_entity = entity;
}

void ComponentsPanel::deselect_entity() {
    m_selected_entity.reset();
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
void render_component(Phos::Entity& entity) {
    if (!entity.has_component<T>())
        return;

    render_component<T>(entity.get_component<T>());
    ImGui::Separator();
}

#define ADD_COMPONENT_POPUP_ITEM(T, name)                                \
    if (!m_selected_entity->has_component<T>() && ImGui::MenuItem(name)) \
    m_selected_entity->add_component<T>()

void ComponentsPanel::on_imgui_render() {
    ImGui::Begin(m_name.c_str());

    if (m_selected_entity.has_value()) {
        auto& entity = *m_selected_entity;

        render_component<Phos::NameComponent>(entity);

        render_component<Phos::TransformComponent>(entity);

        render_component<Phos::MeshRendererComponent>(entity);

        render_component<Phos::LightComponent>(entity);

        // Add component on Right Click
        if (ImGui::Button("Add Component")) {
            ImGui::OpenPopup("##AddComponentWindow");
        }

        if (ImGui::BeginPopup("##AddComponentWindow", ImGuiWindowFlags_MenuBar)) {
            if (ImGui::BeginMenuBar()) {
                ImGui::Text("Add Component");
                ImGui::EndMenuBar();
            }

            ADD_COMPONENT_POPUP_ITEM(Phos::MeshRendererComponent, "Mesh Renderer Component");
            ADD_COMPONENT_POPUP_ITEM(Phos::LightComponent, "Light Component");

            ImGui::EndPopup();
        }
    }

    ImGui::End();
}

//
// Specific render_component
//

template <>
void render_component<Phos::NameComponent>(Phos::NameComponent& component) {
    ImGui::AlignTextToFramePadding();

    ImGui::Text("Name:");
    ImGui::SameLine();
    ImGui::InputText("##NameInput", &component.name);
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

template <>
void render_component<Phos::LightComponent>(Phos::LightComponent& component) {
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Light");

    if (!ImGui::BeginTable("MeshRenderer", 2))
        return;

    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    { ImGui::Text("Type:"); }

    ImGui::TableSetColumnIndex(1);
    {
        constexpr const char* point_light_name = "Point";
        constexpr const char* directional_light_name = "Directional";

        std::string selected_type;

        switch (component.light_type) {
        default:
        case Phos::Light::Type::Point:
            selected_type = point_light_name;
            break;
        case Phos::Light::Type::Directional:
            selected_type = directional_light_name;
            break;
        }

        if (ImGui::BeginCombo("##LightTypeCombo", selected_type.c_str())) {
            if (ImGui::Selectable(point_light_name, component.light_type == Phos::Light::Type::Point))
                component.light_type = Phos::Light::Type::Point;

            if (ImGui::Selectable(directional_light_name, component.light_type == Phos::Light::Type::Directional))
                component.light_type = Phos::Light::Type::Directional;

            ImGui::EndCombo();
        }
    }

    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    { ImGui::Text("Color:"); }

    ImGui::TableSetColumnIndex(1);
    {
        float color[4] = {component.color.x, component.color.y, component.color.z, component.color.w};
        ImGui::ColorEdit4("##LightColorPicker", color, ImGuiColorEditFlags_NoInputs);

        component.color.x = color[0];
        component.color.y = color[1];
        component.color.z = color[2];
        component.color.w = color[3];
    }

    if (component.light_type == Phos::Light::Type::Point) {
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        { ImGui::Text("Radius:"); }

        ImGui::TableSetColumnIndex(1);
        { ImGui::InputFloat("##RadiusInput", &component.radius); }
    }

    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    { ImGui::Text("Shadow Type:"); }

    ImGui::TableSetColumnIndex(1);
    {
        constexpr const char* none_shadow_name = "None";
        constexpr const char* hard_shadow_name = "Hard";

        std::string selected_type;

        switch (component.shadow_type) {
        default:
        case Phos::Light::ShadowType::None:
            selected_type = none_shadow_name;
            break;
        case Phos::Light::ShadowType::Hard:
            selected_type = hard_shadow_name;
            break;
        }

        if (ImGui::BeginCombo("##ShadowTypeCombo", selected_type.c_str())) {
            if (ImGui::Selectable(none_shadow_name, component.shadow_type == Phos::Light::ShadowType::None))
                component.shadow_type = Phos::Light::ShadowType::None;

            if (ImGui::Selectable(hard_shadow_name, component.shadow_type == Phos::Light::ShadowType::Hard))
                component.shadow_type = Phos::Light::ShadowType::Hard;

            ImGui::EndCombo();
        }
    }

    ImGui::EndTable();
}