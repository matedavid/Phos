#include "editor_scene_manager.h"

#include "scene/scene.h"

EditorSceneManager::EditorSceneManager(std::shared_ptr<Phos::Scene> scene) {
    set_scene(std::move(scene));
}

void EditorSceneManager::set_scene(std::shared_ptr<Phos::Scene> scene) {
    m_editing_scene = std::move(scene);
    m_active_scene = m_editing_scene;
}

void EditorSceneManager::running_changed(bool running) {
    if (running)
        m_active_scene = std::make_shared<Phos::Scene>(*m_editing_scene);
    else
        m_active_scene = m_editing_scene;
}

std::shared_ptr<Phos::Scene> EditorSceneManager::active_scene() const {
    return m_active_scene;
}
