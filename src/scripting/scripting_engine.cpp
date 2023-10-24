#include "scripting_engine.h"

#include <fstream>

#include "scripting/class_instance_handle.h"
#include "scripting/class_handle.h"
#include "scripting/script_glue.h"

#include "scene/scene.h"
#include "scene/entity.h"

namespace Phos {

static char* read_file_bytes(const std::filesystem::path& path, uint32_t& file_size) {
    std::ifstream stream(path, std::ios::binary | std::ios::ate);

    if (!stream) {
        // Failed to open the file
        return nullptr;
    }

    std::streampos end = stream.tellg();
    stream.seekg(0, std::ios::beg);
    uint32_t size = end - stream.tellg();

    if (size == 0) {
        // File is empty
        return nullptr;
    }

    char* buffer = new char[size];
    stream.read((char*)buffer, size);
    stream.close();

    file_size = size;
    return buffer;
}

ScriptingEngine::ScriptingEngine(std::filesystem::path dll_path, std::shared_ptr<Scene> scene)
      : m_dll_path(std::move(dll_path)) {
    PS_ASSERT(std::filesystem::exists(m_dll_path), "DLL path '{}' does not exist", m_dll_path.string())

    //
    // Setup mono
    //
    mono_set_assemblies_path("./mono/lib");

    // Initialise mono runtime
    m_root_domain = mono_jit_init("PhosScriptEngine");
    PS_ASSERT(m_root_domain, "Failed to Initialize Mono Runtime")

    const auto load_mono_assembly =
        [](const std::filesystem::path& path, const std::string& name, MonoDomain*& domain, MonoImage*& image) {
            domain = mono_domain_create_appdomain(const_cast<char*>(name.c_str()), nullptr);
            mono_domain_set(domain, true);

            MonoImageOpenStatus status;
            image = mono_image_open_full(path.c_str(), &status, 0);

            if (status != MONO_IMAGE_OK) {
                const char* errorMessage = mono_image_strerror(status);
                PS_FAIL(errorMessage)
            }

            auto* assembly = mono_assembly_load_from_full(image, path.c_str(), &status, 0);
            mono_image_close(image);

            // Create image
            image = mono_assembly_get_image(assembly);
        };

    // Create core domain
    const std::filesystem::path CORE_DLL_PATH = "../ScriptGlue/bin/Debug/PhosEngine.dll";
    load_mono_assembly(CORE_DLL_PATH, "CoreDomain", m_core_domain, m_core_image);

    // Create app domain
    load_mono_assembly(m_dll_path, "AppDomain", m_app_domain, m_app_image);

    //
    // Initialize components
    //
    ScriptGlue::initialize();
    m_entity_class_handle = create_class_handle("PhosEngine", "Entity", m_core_image);

    //
    // Initialize scene
    //
    set_scene(std::move(scene));
}

ScriptingEngine::~ScriptingEngine() {
    ScriptGlue::shutdown();
    mono_jit_cleanup(m_root_domain);
}

void ScriptingEngine::on_update() {
    for (const auto& entity : m_scene->get_entities_with<ScriptComponent>()) {
        auto& sc = entity.get_component<ScriptComponent>();
        if (!sc.is_initialized)
            continue;

        auto& handle = sc.scripting_instance;
        handle->invoke_on_update();
    }
}

void ScriptingEngine::set_scene(std::shared_ptr<Scene> scene) {
    m_scene = std::move(scene);
    ScriptGlue::set_scene(m_scene);

    for (const auto& entity : m_scene->get_entities_with<ScriptComponent>()) {
        auto& sc = entity.get_component<ScriptComponent>();

        sc.scripting_instance = nullptr;
        sc.is_initialized = false;

        auto instance = create_entity_class_instance(entity);
        if (instance == nullptr) {
            PS_ERROR("Failed to create script class instance for class: {}", sc.class_name);
            continue;
        }

        sc.scripting_instance = std::move(instance);
        sc.is_initialized = true;

        sc.scripting_instance->invoke_on_create();
    }
}

std::shared_ptr<ClassHandle> ScriptingEngine::create_class_handle(std::string space,
                                                                  std::string class_name,
                                                                  MonoImage* image) {
    const auto full_name = space.empty() ? class_name : space + "." + class_name;
    if (m_class_handle_cache.contains(class_name)) {
        return m_class_handle_cache[class_name];
    }

    auto* klass = mono_class_from_name(image, space.data(), class_name.data());
    if (klass == nullptr) {
        PS_ERROR("No class found with name: '{}'", full_name);
        return nullptr;
    }

    auto handle = std::make_shared<ClassHandle>(klass);
    m_class_handle_cache[full_name] = handle;

    return handle;
}

std::shared_ptr<ClassInstanceHandle> ScriptingEngine::create_entity_class_instance(const Entity& entity) {
    if (!entity.has_component<ScriptComponent>()) {
        PS_ERROR("Entity must have ScriptComponent to create class instance");
        return nullptr;
    }

    const auto& sc = entity.get_component<ScriptComponent>();

    const auto class_handle = create_class_handle("", sc.class_name, m_app_image);
    if (class_handle == nullptr) {
        PS_ERROR("Could not create class handle for entity: {}", (uint64_t)entity.uuid());
        return nullptr;
    }

    auto* class_instance = mono_object_new(m_app_domain, class_handle->handle());
    if (class_instance == nullptr) {
        PS_ERROR("Failed to create class instance for entity: {}", (uint64_t)entity.uuid());
        return nullptr;
    }

    // Call constructor
    auto* ctor_method = mono_class_get_method_from_name(m_entity_class_handle->handle(), ".ctor", 1);
    PS_ASSERT(ctor_method != nullptr, "No suitable constructor found when creating entity instance")

    void* args[1];
    auto id = (uint64_t)entity.uuid();
    args[0] = &id;

    mono_runtime_invoke(ctor_method, class_instance, args, nullptr);

    return std::make_shared<ClassInstanceHandle>(class_instance, class_handle);
}

} // namespace Phos
