#include "components_panel.h"

#include <misc/cpp/imgui_stdlib.h>
#include "asset_tools/editor_asset.h"
#include "imgui/imgui_utils.h"

#include "scene/scene.h"
#include "asset/editor_asset_manager.h"

#include "renderer/mesh.h"
#include "renderer/backend/material.h"

static std::shared_ptr<Phos::EditorAssetManager> s_asset_manager;

ComponentsPanel::ComponentsPanel(std::string name,
                                 std::shared_ptr<Phos::Scene> scene,
                                 std::shared_ptr<Phos::EditorAssetManager> asset_manager)
      : m_name(std::move(name)), m_scene(std::move(scene)) {
    s_asset_manager = std::move(asset_manager);
}

ComponentsPanel::~ComponentsPanel() {
    s_asset_manager.reset();
}

void ComponentsPanel::set_selected_entity(const Phos::Entity& entity) {
    m_selected_entity = entity;
}

void ComponentsPanel::deselect_entity() {
    m_selected_entity.reset();
}

template <typename T>
void render_label_input(const std::string& label, const std::string& group, T* value, int precision = 1) {
    ImGui::AlignTextToFramePadding();

    ImGui::TextUnformatted(label.c_str());
    ImGui::SameLine();

    const std::string input_label = "##" + group + label;

    if (typeid(T) == typeid(float)) {
        const std::string format_string = "%." + std::to_string(precision) + "f";
        ImGui::InputFloat(input_label.c_str(), value, 0.0f, 0.0f, format_string.c_str());
    } else {
        PS_FAIL("Unsupported input type")
    }
}

template <typename T>
void render_component(T& component);

template <typename T>
void render_component(Phos::Entity& entity, bool right_click = true) {
    if (!entity.has_component<T>())
        return;

    ImGui::BeginGroup();

    render_component<T>(entity.get_component<T>());
    ImGui::Separator();

    ImGui::EndGroup();

    if (!right_click)
        return;

    const std::string id = "##RightClickComponent" + std::string(typeid(T).name());
    if (ImGui::BeginPopupContextItem(id.c_str(), ImGuiPopupFlags_MouseButtonRight)) {
        if (ImGui::MenuItem("Remove Component")) {
            entity.remove_component<T>();
        }

        if (ImGui::MenuItem("Reset Component")) {
            entity.remove_component<T>();
            entity.add_component<T>({});
        }

        ImGui::EndPopup();
    }
}

#define ADD_COMPONENT_POPUP_ITEM(T, name)                                \
    if (!m_selected_entity->has_component<T>() && ImGui::MenuItem(name)) \
    m_selected_entity->add_component<T>()

void ComponentsPanel::on_imgui_render() {
    ImGui::Begin(m_name.c_str());

    if (m_selected_entity.has_value()) {
        auto& entity = *m_selected_entity;

        // Components
        render_component<Phos::NameComponent>(entity, false);
        render_component<Phos::TransformComponent>(entity, false);
        render_component<Phos::MeshRendererComponent>(entity);
        render_component<Phos::LightComponent>(entity);
        render_component<Phos::CameraComponent>(entity);

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
            ADD_COMPONENT_POPUP_ITEM(Phos::CameraComponent, "Camera Component");

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

    auto mesh_name = component.mesh != nullptr ? component.mesh->asset_name : "";
    ImGui::InputText("##MeshInput", mesh_name.data(), mesh_name.length(), ImGuiInputTextFlags_ReadOnly);

    const auto mesh_asset = ImGuiUtils::drag_drop_target<EditorAsset>("CONTENT_BROWSER_ITEM");
    if (mesh_asset.has_value() && !mesh_asset->is_directory && mesh_asset->type == Phos::AssetType::Mesh)
        component.mesh = s_asset_manager->load_by_id_type<Phos::Mesh>(mesh_asset->uuid);

    ImGui::TableNextRow();

    // Material
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Material:");

    ImGui::TableSetColumnIndex(1);

    auto material_name = component.material != nullptr ? component.material->asset_name : "";
    ImGui::InputText("##MaterialInput", material_name.data(), material_name.length(), ImGuiInputTextFlags_ReadOnly);

    const auto mat_asset = ImGuiUtils::drag_drop_target<EditorAsset>("CONTENT_BROWSER_ITEM");
    if (mat_asset.has_value() && !mat_asset->is_directory && mat_asset->type == Phos::AssetType::Material)
        component.material = s_asset_manager->load_by_id_type<Phos::Material>(mat_asset->uuid);

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

        switch (component.type) {
        default:
        case Phos::Light::Type::Point:
            selected_type = point_light_name;
            break;
        case Phos::Light::Type::Directional:
            selected_type = directional_light_name;
            break;
        }

        if (ImGui::BeginCombo("##LightTypeCombo", selected_type.c_str())) {
            if (ImGui::Selectable(point_light_name, component.type == Phos::Light::Type::Point))
                component.type = Phos::Light::Type::Point;

            if (ImGui::Selectable(directional_light_name, component.type == Phos::Light::Type::Directional))
                component.type = Phos::Light::Type::Directional;

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

    if (component.type == Phos::Light::Type::Point) {
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

template <>
void render_component<Phos::CameraComponent>(Phos::CameraComponent& component) {
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Camera");

    if (!ImGui::BeginTable("Camera Component", 2))
        return;

    ImGui::TableNextRow();

    // Type
    ImGui::TableSetColumnIndex(0);
    { ImGui::Text("Type:"); }

    ImGui::TableSetColumnIndex(1);
    {
        constexpr const char* perspective_camera_name = "Perspective";
        constexpr const char* orthographic_camera_name = "Orthographic";

        std::string selected_type;

        switch (component.type) {
        default:
        case Phos::Camera::Type::Perspective:
            selected_type = perspective_camera_name;
            break;
        case Phos::Camera::Type::Orthographic:
            selected_type = orthographic_camera_name;
            break;
        }

        if (ImGui::BeginCombo("##CameraTypeCombo", selected_type.c_str())) {
            if (ImGui::Selectable(perspective_camera_name, component.type == Phos::Camera::Type::Perspective))
                component.type = Phos::Camera::Type::Perspective;

            /*
            if (ImGui::Selectable(orthographic_camera_name, component.type == Phos::Camera::Type::Orthographic))
                component.type = Phos::Camera::Type::Orthographic;
            */

            ImGui::EndCombo();
        }
    }

    // Fov (Perspective) and Size (Orthographic)
    if (component.type == Phos::Camera::Type::Perspective) {
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        { ImGui::Text("Fov:"); }

        ImGui::TableSetColumnIndex(1);
        { ImGui::InputFloat("##FovInput", &component.fov); }
    } else if (component.type == Phos::Camera::Type::Orthographic) {
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        { ImGui::Text("Size:"); }

        ImGui::TableSetColumnIndex(1);
        { ImGui::InputFloat("##OrthographicSizeInput", &component.size); }
    }

    ImGui::TableNextRow();

    // znear / zfar
    ImGui::TableSetColumnIndex(0);
    render_label_input("znear", "CameraZnear", &component.znear, 3);

    ImGui::TableSetColumnIndex(1);
    render_label_input("zfar", "CameraZfar", &component.zfar, 2);

    ImGui::TableNextRow();

    // Depth
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Depth:");

    ImGui::TableSetColumnIndex(1);
    ImGui::InputInt("##DepthCamera", &component.depth);

    ImGui::EndTable();
}
