#include "scripting_engine.h"

#include <fstream>

#include "scripting/class_handle.h"
#include "scripting/script_glue.h"

namespace Phos {

MonoDomain* ScriptingEngine::m_root_domain = nullptr;
ScriptingEngine::ScriptingEngineContext ScriptingEngine::m_context;

void ScriptingEngine::initialize() {
    mono_set_assemblies_path("./mono/lib");

    // Initialize mono runtime and root domain
    m_root_domain = mono_jit_init("PhosScriptEngine");
    PS_ASSERT(m_root_domain, "Failed to Initialize Mono Runtime")

    // Load Core domain
    const std::filesystem::path CORE_DLL_PATH = "../ScriptGlue/bin/Debug/PhosEngine.dll"; // @TODO: Careful with path!
    load_mono_assembly(CORE_DLL_PATH, "CoreDomain", m_context.core_domain, m_context.core_image);

    m_context.entity_class_handle = ClassHandle::create("PhosEngine", "Entity");

    // Initialize ScriptGlue
    ScriptGlue::initialize();
}

void ScriptingEngine::shutdown() {
    ScriptGlue::shutdown();
    mono_jit_cleanup(m_root_domain);
}

void ScriptingEngine::set_dll_path(const std::filesystem::path& dll_path) {
    if (!std::filesystem::exists(dll_path)) {
        PS_ERROR("dll file path does not exist: '{}'", dll_path.string());
        return;
    }

    if (m_context.app_domain != nullptr) {
        const auto name = mono_domain_get_friendly_name(m_context.app_domain);
        PS_INFO("Unloading App Domain: '{}'", name);

        m_context.klass_cache.clear();

        // @TODO: Don't like this, i'm not sure that everything gets unloaded correctly
        mono_image_close(m_context.app_image);
    }

    // Create app domain
    load_mono_assembly(dll_path, "AppDomain", m_context.app_domain, m_context.app_image);
}

void ScriptingEngine::load_mono_assembly(const std::filesystem::path& path,
                                         const std::string& name,
                                         MonoDomain*& domain,
                                         MonoImage*& image) {
    domain = mono_domain_create_appdomain(const_cast<char*>(name.c_str()), nullptr);
    mono_domain_set(domain, true);

    MonoImageOpenStatus status;
    image = mono_image_open_full(path.c_str(), &status, 0);

    if (status != MONO_IMAGE_OK) {
        const char* errorMessage = mono_image_strerror(status);
        PS_FAIL("Mono error while opening image: {}", errorMessage)
    }

    auto* assembly = mono_assembly_load_from_full(image, path.c_str(), &status, 0);
    mono_image_close(image);

    // Create image
    image = mono_assembly_get_image(assembly);
}

} // namespace Phos
