#pragma once

#include <filesystem>
#include <unordered_map>

#include "core/uuid.h"

namespace Phos {

// Forward declarations
class Project;
class Scene;
class Entity;
class ClassInstanceHandle;

class ScriptingSystem {
  public:
    explicit ScriptingSystem(std::shared_ptr<Project> project);
    ~ScriptingSystem() = default;

    void on_update(double ts);

    void start(std::shared_ptr<Scene> scene);
    void shutdown();

  private:
    std::shared_ptr<Project> m_project;
    std::filesystem::path m_dll_path;

    std::shared_ptr<Scene> m_scene_copy;

    std::unordered_map<UUID, std::shared_ptr<ClassInstanceHandle>> m_entity_script_instance;

    [[nodiscard]] std::shared_ptr<ClassInstanceHandle> create_entity_instance(const Entity& entity);
};

} // namespace Phos
