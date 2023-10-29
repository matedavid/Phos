#pragma once

#include "core.h"

#include <filesystem>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace Phos {

// Forward declarations
class Entity;
class Scene;
class AssetManagerBase;
class ScriptComponent;
class ClassHandle;
class ClassInstanceHandle;

class ScriptingEngine {
  public:
    explicit ScriptingEngine(std::filesystem::path dll_path,
                             std::shared_ptr<Scene> scene,
                             std::shared_ptr<AssetManagerBase> asset_manager);
    ~ScriptingEngine();

    void on_update();

    void set_scene(std::shared_ptr<Scene> scene);

  private:
    MonoDomain* m_root_domain{};

    MonoDomain* m_core_domain{};
    MonoImage* m_core_image{};

    MonoDomain* m_app_domain{};
    MonoImage* m_app_image{};

    std::shared_ptr<Scene> m_scene;
    std::shared_ptr<AssetManagerBase> m_asset_manager;

    std::filesystem::path m_dll_path;

    std::shared_ptr<ClassHandle> m_entity_class_handle;
    std::unordered_map<std::string, std::shared_ptr<ClassHandle>> m_class_handle_cache;

    [[nodiscard]] std::shared_ptr<ClassHandle> create_class_handle(std::string space,
                                                                   std::string class_name,
                                                                   MonoImage* image);
    [[nodiscard]] std::shared_ptr<ClassInstanceHandle> create_entity_class_instance(const Entity& entity);
};

} // namespace Phos
