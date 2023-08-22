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

    ImGui::End();
}

void EntityHierarchyPanel::render_entity_r(const Phos::Entity& entity) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;

    const auto name = entity.get_component<Phos::NameComponent>().name;
    const auto children = entity.get_component<Phos::RelationshipComponent>().children;
    if (children.empty())
        flags |= ImGuiTreeNodeFlags_Leaf;

    if (ImGui::TreeNodeEx(name.c_str(), flags)) {
        for (const auto& child_uuid : children) {
            const auto child = m_scene->get_entity_with_uuid(child_uuid);
            render_entity_r(child);
        }

        ImGui::TreePop();
    }
}