#pragma once

#include "imgui_panel.h"

#include "scene/entity.h"

namespace Phos {

// Forward declarations
class Scene;

} // namespace Phos

class ComponentsPanel : public IImGuiPanel {
  public:
    ComponentsPanel(std::string name, std::shared_ptr<Phos::Scene> scene);
    ~ComponentsPanel() override = default;

    void on_imgui_render() override;
    void set_selected_entity(const Phos::Entity& entity);

  private:
    std::string m_name;
    std::shared_ptr<Phos::Scene> m_scene;

    std::optional<Phos::Entity> m_selected_entity;
};
