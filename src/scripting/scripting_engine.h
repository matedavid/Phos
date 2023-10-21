#pragma once

#include "core.h"

#include <filesystem>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace Phos {

// Forward declarations
class Entity;
class Scene;
class ScriptComponent;
class ClassInstanceHandle;

class ScriptingEngine {
  public:
    explicit ScriptingEngine(std::filesystem::path dll_path, std::shared_ptr<Scene> scene);
    ~ScriptingEngine();

    void on_update();

    void set_scene(std::shared_ptr<Scene> scene);

  private:
    MonoDomain* m_root_domain;
    MonoDomain* m_app_domain;
    MonoImage* m_image;

    std::shared_ptr<Scene> m_scene = nullptr;
    std::filesystem::path m_dll_path;

    std::shared_ptr<ClassInstanceHandle> create_class_instance(std::string_view class_name);
};

} // namespace Phos
