#pragma once

#include "imgui_panel.h"

#include "scene/entity.h"

namespace Phos {

// Forward declarations
class Scene;

} // namespace Phos

class EntityHierarchyPanel : public IImGuiPanel {
  public:
    EntityHierarchyPanel(std::string name, std::shared_ptr<Phos::Scene> m_scene);
    ~EntityHierarchyPanel() override = default;

    void on_imgui_render() override;
    [[nodiscard]] std::optional<Phos::Entity> get_selected_entity() const { return m_selected_entity; }

  private:
    std::string m_name;
    std::shared_ptr<Phos::Scene> m_scene;

    std::optional<Phos::Entity> m_selected_entity;

    void render_entity_r(const Phos::Entity& entity);
    void select_entity(const Phos::Entity& entity);
};
