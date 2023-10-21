#include "scripting_engine.h"

#include <fstream>

#include "scripting/class_instance_handle.h"

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

    // Create app domain
    m_app_domain = mono_domain_create_appdomain(const_cast<char*>("PhosScriptDomain"), nullptr);
    mono_domain_set(m_app_domain, true);

    // Load assembly
    uint32_t file_size;
    auto* file_data = read_file_bytes(m_dll_path, file_size);
    PS_ASSERT(file_data != nullptr, "Could not open file: {}", m_dll_path.string())

    MonoImageOpenStatus status;
    auto* image = mono_image_open_from_data_full(file_data, file_size, 1, &status, 0);

    if (status != MONO_IMAGE_OK) {
        const char* errorMessage = mono_image_strerror(status);
        PS_FAIL(errorMessage)
    }

    auto* assembly = mono_assembly_load_from_full(image, m_dll_path.c_str(), &status, 0);
    mono_image_close(image);

    delete[] file_data;

    // Create image
    m_image = mono_assembly_get_image(assembly);

    //
    // Initialize scene
    //
    set_scene(std::move(scene));
}

ScriptingEngine::~ScriptingEngine() {
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

    for (const auto& entity : m_scene->get_entities_with<ScriptComponent>()) {
        auto& sc = entity.get_component<ScriptComponent>();
        sc.is_initialized = false;

        auto instance = create_class_instance(sc.class_name);
        if (instance == nullptr) {
            PS_ERROR("Failed to create script class instance for class: {}", sc.class_name);
            continue;
        }

        sc.scripting_instance = std::move(instance);
        sc.is_initialized = true;

        sc.scripting_instance->invoke_on_create();
    }
}

std::shared_ptr<ClassInstanceHandle> ScriptingEngine::create_class_instance(std::string_view class_name) {
    auto* klass = mono_class_from_name(m_image, "", class_name.data());
    auto* class_instance = mono_object_new(m_app_domain, klass);
    if (class_instance == nullptr) {
        return nullptr;
    }
    mono_runtime_object_init(class_instance);

    return std::make_shared<ClassInstanceHandle>(klass, class_instance);
}

} // namespace Phos
