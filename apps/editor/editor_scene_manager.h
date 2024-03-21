#pragma once

#include <memory>

namespace Phos {

// Forward declarations
class Scene;

} // namespace Phos

class EditorSceneManager {
  public:
    explicit EditorSceneManager(std::shared_ptr<Phos::Scene> scene);

    void set_scene(std::shared_ptr<Phos::Scene> scene);
    void running_changed(bool running);

    [[nodiscard]] std::shared_ptr<Phos::Scene> active_scene() const { return m_active_scene; }
    [[nodiscard]] std::shared_ptr<Phos::Scene> editing_scene() const { return m_editing_scene; }

  private:
    std::shared_ptr<Phos::Scene> m_editing_scene;
    std::shared_ptr<Phos::Scene> m_active_scene;
};
