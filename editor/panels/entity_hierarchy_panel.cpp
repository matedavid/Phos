#include "entity_hierarchy_panel.h"

#include "scene/scene.h"
#include "scene/entity.h"

EntityHierarchyPanel::EntityHierarchyPanel(std::string name, std::shared_ptr<Phos::Scene> scene)
      : m_name(std::move(name)), m_scene(std::move(scene)) {}

void EntityHierarchyPanel::on_imgui_render() {
    ImGui::Begin(m_name.c_str());

    std::vector<Phos::Entity> parent_entities;
    std::ranges::copy_if(m_scene->get_entities_with<Phos::RelationshipComponent>(),
                         std::back_inserter(parent_entities),
                         [](Phos::Entity& entity) {
                             auto relationship = entity.get_component<Phos::RelationshipComponent>();
                             return !relationship.parent.has_value();
                         });

    for (const auto& entity : parent_entities) {
        render_entity_r(entity);
    }

    // Left click on blank = deselect entity
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()) {
        m_selected_entity.reset();
    }

    // Right click on blank
    if (ImGui::BeginPopupContextWindow("##RightClickEntityHierarchy", ImGuiPopupFlags_MouseButtonRight)) {
        if (ImGui::MenuItem("Create Empty Entity")) {
            m_scene->create_entity();
        }

        ImGui::EndPopup();
    }

    ImGui::End();
}

void EntityHierarchyPanel::render_entity_r(const Phos::Entity& entity) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (m_selected_entity.has_value() && m_selected_entity.value().uuid() == entity.uuid())
        flags |= ImGuiTreeNodeFlags_Selected;

    const auto& name = entity.get_component<Phos::NameComponent>().name;
    const auto& children = entity.get_component<Phos::RelationshipComponent>().children;
    if (children.empty())
        flags |= ImGuiTreeNodeFlags_Leaf;

    const bool opened = ImGui::TreeNodeEx((void*)(uint64_t)entity.uuid(), flags, name.c_str());
    if (ImGui::IsItemClicked()) {
        select_entity(entity);
    }

    if (opened) {
        for (const auto& child_uuid : children) {
            const auto& child = m_scene->get_entity_with_uuid(child_uuid);
            render_entity_r(child);
        }

        ImGui::TreePop();
    }
}

void EntityHierarchyPanel::select_entity(const Phos::Entity& entity) {
    m_selected_entity = entity;
}
