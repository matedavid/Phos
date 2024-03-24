#include "entity_components_renderer.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include "imgui/imgui_utils.h"
#include "asset_tools/editor_asset.h"

#include "utility/logging.h"

#include "scene/scene.h"
#include "asset/editor_asset_manager.h"

#include "renderer/mesh.h"
#include "renderer/backend/material.h"

#include "scripting/class_handle.h"

template <typename T>
void render_label_input(const std::string& label, const std::string& group, T* value, int precision = 3) {
    ImGui::AlignTextToFramePadding();

    ImGui::TextUnformatted(label.c_str());
    ImGui::SameLine();

    const std::string input_label = "##" + group + label;

    if (typeid(T) == typeid(float)) {
        const std::string format_string = "%." + std::to_string(precision) + "f";
        ImGui::InputFloat(input_label.c_str(), value, 0.0f, 0.0f, format_string.c_str());
    } else {
        PHOS_FAIL("Unsupported input type");
    }
}

template <typename T>
void render_component(T& component,
                      const std::shared_ptr<Phos::Scene>& scene,
                      const std::shared_ptr<Phos::EditorAssetManager>& asset_manager);

template <typename T>
void render_component(Phos::Entity& entity,
                      const std::shared_ptr<Phos::Scene>& scene,
                      const std::shared_ptr<Phos::EditorAssetManager>& asset_manager,
                      bool right_click = true) {
    if (!entity.has_component<T>())
        return;

    ImGui::BeginGroup();

    render_component<T>(entity.get_component<T>(), scene, asset_manager);
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

#define ADD_COMPONENT_POPUP_ITEM(T, name)                    \
    if (!entity.has_component<T>() && ImGui::MenuItem(name)) \
    entity.add_component<T>()

void EntityComponentsRenderer::display(Phos::Entity& entity,
                                       const std::shared_ptr<Phos::Scene>& scene,
                                       const std::shared_ptr<Phos::EditorAssetManager>& asset_manager) {
    // Components
    render_component<Phos::NameComponent>(entity, scene, asset_manager, false);
    render_component<Phos::TransformComponent>(entity, scene, asset_manager, false);
    render_component<Phos::MeshRendererComponent>(entity, scene, asset_manager);
    render_component<Phos::LightComponent>(entity, scene, asset_manager);
    render_component<Phos::CameraComponent>(entity, scene, asset_manager);
    render_component<Phos::ScriptComponent>(entity, scene, asset_manager);

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
        ADD_COMPONENT_POPUP_ITEM(Phos::ScriptComponent, "Script Component");

        ImGui::EndPopup();
    }
}

//
// Specific render_component
//

#define RENDER_COMPONENT(T)                                                              \
    template <>                                                                          \
    void render_component<T>(T & component,                                              \
                             [[maybe_unused]] const std::shared_ptr<Phos::Scene>& scene, \
                             [[maybe_unused]] const std::shared_ptr<Phos::EditorAssetManager>& asset_manager)

RENDER_COMPONENT(Phos::NameComponent) {
    ImGui::AlignTextToFramePadding();

    ImGui::Text("Name:");
    ImGui::SameLine();
    ImGui::InputText("##NameInput", &component.name);
}

RENDER_COMPONENT(Phos::TransformComponent) {
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

    auto rotation_degrees = glm::degrees(component.rotation);

    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Rotation:");

    ImGui::TableSetColumnIndex(1);
    render_label_input("X", "Rotation", &rotation_degrees.x);

    ImGui::TableSetColumnIndex(2);
    render_label_input("Y", "Rotation", &rotation_degrees.y);

    ImGui::TableSetColumnIndex(3);
    render_label_input("Z", "Rotation", &rotation_degrees.z);

    component.rotation = glm::radians(rotation_degrees);

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

RENDER_COMPONENT(Phos::MeshRendererComponent) {
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
        component.mesh = asset_manager->load_by_id_type<Phos::Mesh>(mesh_asset->uuid);

    ImGui::TableNextRow();

    // Material
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Material:");

    ImGui::TableSetColumnIndex(1);

    auto material_name = component.material != nullptr ? component.material->asset_name : "";
    ImGui::InputText("##MaterialInput", material_name.data(), material_name.length(), ImGuiInputTextFlags_ReadOnly);

    const auto mat_asset = ImGuiUtils::drag_drop_target<EditorAsset>("CONTENT_BROWSER_ITEM");
    if (mat_asset.has_value() && !mat_asset->is_directory && mat_asset->type == Phos::AssetType::Material)
        component.material = asset_manager->load_by_id_type<Phos::Material>(mat_asset->uuid);

    ImGui::EndTable();
}

RENDER_COMPONENT(Phos::LightComponent) {
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

    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    { ImGui::Text("Intensity:"); }

    ImGui::TableSetColumnIndex(1);
    { ImGui::InputFloat("##IntensityInput", &component.intensity); }

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

RENDER_COMPONENT(Phos::CameraComponent) {
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

RENDER_COMPONENT(Phos::ScriptComponent) {
    ImGui::Text("Script");

    if (!ImGui::BeginTable("Script", 2))
        return;

    ImGui::TableSetupColumn("##Column1", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("##Column2", ImGuiTableColumnFlags_WidthStretch);

    ImGui::TableNextRow();

    ImGui::AlignTextToFramePadding();

    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Script file");

    ImGui::TableSetColumnIndex(1);
    ImGui::InputText(
        "##ScriptFileInput", component.class_name.data(), component.class_name.length(), ImGuiInputTextFlags_ReadOnly);
    const auto asset = ImGuiUtils::drag_drop_target<EditorAsset>("CONTENT_BROWSER_ITEM");
    if (asset.has_value() && !asset->is_directory && asset->type == Phos::AssetType::Script) {
        component.script = asset->uuid;

        const auto handle = asset_manager->load_by_id_type<Phos::ClassHandle>(component.script);
        component.class_name = handle->class_name();
        component.field_values = {};

        for (const auto& [name, _, type] : handle->get_all_fields()) {
            component.field_values[name] = Phos::ClassField::get_default_value(type);
        }
    }

    if (component.field_values.empty()) {
        ImGui::EndTable();
        return;
    }

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Fields:");

    for (const auto& [name, value] : component.field_values) {
        ImGui::TableNextRow();
        ImGui::AlignTextToFramePadding();

        const auto id = "##Field" + name;

        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%s:", name.c_str());

        ImGui::TableSetColumnIndex(1);

        if (std::holds_alternative<int32_t>(value)) {
            auto& v = const_cast<int32_t&>(std::get<int32_t>(value));
            ImGui::InputInt(id.c_str(), &v, 0, 0);
        } else if (std::holds_alternative<float>(value)) {
            auto& v = const_cast<float&>(std::get<float>(value));
            ImGui::InputFloat(id.c_str(), &v, 0, 0);
        } else if (std::holds_alternative<glm::vec3>(value)) {
            if (ImGui::BeginTable(id.c_str(), 3)) {
                ImGui::TableNextRow();

                auto& v = const_cast<glm::vec3&>(std::get<glm::vec3>(value));

                ImGui::TableSetColumnIndex(0);
                render_label_input("X", id + "x", &v.x);

                ImGui::TableSetColumnIndex(1);
                render_label_input("Y", id + "y", &v.y);

                ImGui::TableSetColumnIndex(2);
                render_label_input("Z", id + "z", &v.y);

                ImGui::EndTable();
            }

        } else if (std::holds_alternative<std::string>(value)) {
            auto& v = const_cast<std::string&>(std::get<std::string>(value));
            ImGui::InputText(id.c_str(), &v);
        } else if (std::holds_alternative<Phos::PrefabRef>(value)) {
            auto& v = const_cast<Phos::PrefabRef&>(std::get<Phos::PrefabRef>(value));
            auto prefab_name = v.id == Phos::UUID(0) ? "" : *asset_manager->get_asset_name(v.id);
            ImGui::InputText(id.c_str(), &prefab_name, ImGuiInputTextFlags_ReadOnly);

            const auto prefab_asset = ImGuiUtils::drag_drop_target<EditorAsset>("CONTENT_BROWSER_ITEM");
            if (prefab_asset.has_value() && !prefab_asset->is_directory &&
                prefab_asset->type == Phos::AssetType::Prefab) {
                v.id = prefab_asset->uuid;
            }
        } else if (std::holds_alternative<Phos::EntityRef>(value)) {
            auto& v = const_cast<Phos::EntityRef&>(std::get<Phos::EntityRef>(value));

            auto entity_name = v.id == Phos::UUID(0)
                                   ? ""
                                   : scene->get_entity_with_uuid(v.id).get_component<Phos::NameComponent>().name;
            ImGui::InputText(id.c_str(), &entity_name, ImGuiInputTextFlags_ReadOnly);

            const auto entity_drag = ImGuiUtils::drag_drop_target<Phos::UUID>("ENTITY_HIERARCHY_ITEM");
            if (entity_drag.has_value() && *entity_drag != Phos::UUID(0)) {
                v.id = *entity_drag;
            }
        }
    }

    ImGui::EndTable();
}
