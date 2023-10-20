#pragma once

#include "core.h"

#include <filesystem>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#include "core/uuid.h"

namespace Phos {

// Forward declarations
class Entity;

class ScriptingEngine {
  public:
    explicit ScriptingEngine(std::filesystem::path dll_path);
    ~ScriptingEngine();

    void on_update();

    void initialize_script_entity(const Entity& entity);

  private:
    MonoDomain* m_root_domain;
    MonoDomain* m_app_domain;
    MonoImage* m_image;

    std::filesystem::path m_dll_path;

    struct MonoClassReference {
        MonoClass* klass;
        MonoObject* class_instance;
    };

    std::unordered_map<UUID, MonoClassReference> m_entity_to_class_ref;
};

} // namespace Phos
