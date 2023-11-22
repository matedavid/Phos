#include "entity_hierarchy_panel.h"

#include "panels/content_browser_panel.h"

#include "editor_scene_manager.h"

#include "scene/scene.h"
#include "scene/entity.h"

EntityHierarchyPanel::EntityHierarchyPanel(std::string name,
                                           std::shared_ptr<EditorSceneManager> scene_manager,
                                           ContentBrowserPanel* content_browser_panel)
      : m_name(std::move(name)), m_scene_manager(std::move(scene_manager)),
        m_content_browser_panel(content_browser_panel) {}

void EntityHierarchyPanel::on_imgui_render() {
    ImGui::Begin(m_name.c_str());

    std::vector<Phos::Entity> parent_entities;
    std::ranges::copy_if(m_scene_manager->active_scene()->get_entities_with<Phos::RelationshipComponent>(),
                         std::back_inserter(parent_entities),
                         [](Phos::Entity& entity) {
                             auto relationship = entity.get_component<Phos::RelationshipComponent>();
                             return !relationship.parent.has_value();
                         });

    bool entity_hovered = false;
    for (const auto& entity : parent_entities) {
        entity_hovered |= render_entity_r(entity);
    }

    // Left click on blank = deselect entity
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()) {
        m_selected_entity.reset();
    }

    // Right click on blank
    if (!entity_hovered &&
        ImGui::BeginPopupContextWindow("##RightClickEntityHierarchy", ImGuiPopupFlags_MouseButtonRight)) {
        if (ImGui::MenuItem("Create Empty Entity")) {
            m_scene_manager->active_scene()->create_entity();
        }

        ImGui::EndPopup();
    }

    ImGui::End();
}

bool EntityHierarchyPanel::render_entity_r(const Phos::Entity& entity) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (m_selected_entity.has_value() && m_selected_entity.value().uuid() == entity.uuid())
        flags |= ImGuiTreeNodeFlags_Selected;

    const auto& name = entity.get_component<Phos::NameComponent>().name;
    const auto& children = entity.get_component<Phos::RelationshipComponent>().children;
    if (children.empty())
        flags |= ImGuiTreeNodeFlags_Leaf;

    const bool opened = ImGui::TreeNodeEx((void*)static_cast<uint64_t>(entity.uuid()), flags, "%s", name.c_str());
    bool hovered = ImGui::IsItemHovered();
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        select_entity(entity);
    }

    bool delete_entity = false;
    if (ImGui::BeginPopupContextItem()) {
        select_entity(entity);

        if (ImGui::MenuItem("Create Child Entity")) {
            const auto child = m_scene_manager->active_scene()->create_entity();
            entity.add_child(child);
        }

        if (ImGui::MenuItem("Remove Entity")) {
            delete_entity = true;
        }

        if (ImGui::MenuItem("Create Prefab")) {
            m_content_browser_panel->create_prefab(entity.get_component<Phos::NameComponent>().name, entity);
        }

        ImGui::EndPopup();
    }

    if (opened) {
        for (const auto& child_uuid : children) {
            const auto& child = m_scene_manager->active_scene()->get_entity_with_uuid(child_uuid);
            hovered |= render_entity_r(child);
        }

        ImGui::TreePop();
    }

    if (delete_entity) {
        if (m_selected_entity == entity)
            m_selected_entity = {};

        m_scene_manager->active_scene()->destroy_entity(entity);
    }

    return hovered;
}

void EntityHierarchyPanel::select_entity(const Phos::Entity& entity) {
    m_selected_entity = entity;
}
