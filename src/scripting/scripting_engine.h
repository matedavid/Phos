#pragma once

#include "core.h"

#include <filesystem>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace Phos {

// Forward declarations
class ClassHandle;
class ClassInstanceHandle;

class ScriptingEngine {
  public:
    ScriptingEngine() = delete;

    static void initialize();
    static void shutdown();

    static void set_dll_path(const std::filesystem::path& dll_path);

  private:
    static bool load_mono_assembly(const std::filesystem::path& path,
                                   const std::string& name,
                                   MonoDomain*& domain,
                                   MonoAssembly*& assembly,
                                   MonoImage*& image);

    static MonoDomain* m_root_domain;

    struct ScriptingEngineContext {
        MonoDomain* core_domain = nullptr;
        MonoAssembly* core_assembly = nullptr;
        MonoImage* core_image = nullptr;

        MonoDomain* app_domain = nullptr;
        MonoAssembly* app_assembly = nullptr;
        MonoImage* app_image = nullptr;

        std::shared_ptr<ClassHandle> entity_class_handle;
        std::unordered_map<std::string, std::shared_ptr<ClassHandle>> klass_cache;
    };

    static ScriptingEngineContext m_context;

    friend class ClassHandle;
    friend class ClassInstanceHandle;
};

} // namespace Phos
