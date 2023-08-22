#pragma once

#include <imgui.h>
#include <imgui_internal.h>

#include "imgui_panel.h"

namespace Phos {

// Forward declarations
class Scene;
class Entity;

} // namespace Phos

class EntityHierarchyPanel : public IImGuiPanel {
  public:
    EntityHierarchyPanel(std::string name, std::shared_ptr<Phos::Scene> m_scene);
    ~EntityHierarchyPanel() override = default;

    void on_imgui_render() override;

  private:
    std::string m_name;
    std::shared_ptr<Phos::Scene> m_scene;

    void render_entity_r(const Phos::Entity& entity);
};
