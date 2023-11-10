#pragma once

#include "core.h"

namespace Phos {

// Forward declarations
class Scene;

} // namespace Phos

class EditorSceneManager {
  public:
    explicit EditorSceneManager(std::shared_ptr<Phos::Scene> scene);

    void set_scene(std::shared_ptr<Phos::Scene> scene);
    void running_changed(bool running);

    [[nodiscard]] std::shared_ptr<Phos::Scene> active_scene() const;

  private:
    std::shared_ptr<Phos::Scene> m_editing_scene;
    std::shared_ptr<Phos::Scene> m_active_scene;
};
