#pragma once

#include "imgui_panel.h"

#include "scene/entity.h"

namespace Phos {

// Forward declarations
class Scene;

} // namespace Phos

// Forward declarations
class EditorSceneManager;
class ContentBrowserPanel;

class EntityHierarchyPanel : public IImGuiPanel {
  public:
    EntityHierarchyPanel(std::string name,
                         std::shared_ptr<EditorSceneManager> scene_manager,
                         ContentBrowserPanel* content_browser_panel);
    ~EntityHierarchyPanel() override = default;

    void on_imgui_render() override;
    [[nodiscard]] std::optional<Phos::Entity> get_selected_entity() const { return m_selected_entity; }
    void clear_selected_entity() { m_selected_entity = {}; }

  private:
    std::string m_name;
    std::shared_ptr<EditorSceneManager> m_scene_manager;

    ContentBrowserPanel* m_content_browser_panel;

    std::optional<Phos::Entity> m_selected_entity;

    bool render_entity_r(const Phos::Entity& entity);
    void select_entity(const Phos::Entity& entity);
};
