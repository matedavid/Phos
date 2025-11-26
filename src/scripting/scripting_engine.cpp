#include "scripting_engine.h"

#include <fstream>

#include "scripting/class_handle.h"
#include "scripting/script_glue.h"
#include "utility/logging.h"

#ifndef PHOS_MONO_LIB_PATH
#error Mono library path must be defined in order to use scripting
#endif

namespace Phos {

MonoDomain* ScriptingEngine::m_root_domain = nullptr;
ScriptingEngine::ScriptingEngineContext ScriptingEngine::m_context;

void ScriptingEngine::initialize() {
    mono_set_assemblies_path(PHOS_MONO_LIB_PATH);

    // Initialize mono runtime and root domain
    m_root_domain = mono_jit_init("PhosScriptingEngine");
    PHOS_ASSERT(m_root_domain, "Failed to Initialize Mono Runtime");

    // Load Core domain
    const std::filesystem::path CORE_DLL_PATH =
        "../../../ScriptGlue/bin/Debug/PhosEngine.dll"; // @TODO: Careful with path!

    [[maybe_unused]] const auto loaded = load_mono_assembly(
        CORE_DLL_PATH, "PhosCoreDomain", m_context.core_domain, m_context.core_assembly, m_context.core_image);
    PHOS_ASSERT(loaded, "Failed to load Core Assembly");

    m_context.entity_class_handle = ClassHandle::create("PhosEngine", "ScriptableEntity");

    // Initialize ScriptGlue
    ScriptGlue::initialize();
}

void ScriptingEngine::shutdown() {
    ScriptGlue::shutdown();
    mono_jit_cleanup(m_root_domain);
}

void ScriptingEngine::set_dll_path(const std::filesystem::path& dll_path) {
    if (!std::filesystem::exists(dll_path)) {
        PHOS_LOG_ERROR("dll file path does not exist: '{}'", dll_path.string());
        return;
    }

    if (m_context.app_domain != nullptr) {
        const auto name = mono_domain_get_friendly_name(m_context.app_domain);
        PHOS_LOG_INFO("Unloading App Domain: '{}'", name);

        m_context.klass_cache.clear();
        mono_domain_unload(m_context.app_domain);
    }

    // Create app domain
    load_mono_assembly(dll_path, "PhosAppDomain", m_context.app_domain, m_context.app_assembly, m_context.app_image);
}

bool ScriptingEngine::load_mono_assembly(const std::filesystem::path& path,
                                         const std::string& name,
                                         MonoDomain*& domain,
                                         MonoAssembly*& assembly,
                                         MonoImage*& image) {
    // Create domain
    domain = mono_domain_create_appdomain(const_cast<char*>(name.c_str()), nullptr);
    if (!domain)
        return false;

    // Load assembly
    auto s = path.string();
    assembly = mono_domain_assembly_open(domain, s.c_str());
    if (!assembly)
        return false;

    // Create image
    image = mono_assembly_get_image(assembly);
    if (!image)
        return false;

    return true;
}

} // namespace Phos
