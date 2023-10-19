#include <iostream>
#include <fstream>
#include <filesystem>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

// Global variables, for example purposes
MonoDomain* rootDomain = nullptr;
MonoDomain* appDomain = nullptr;

char* ReadBytes(const std::string& filepath, uint32_t* outSize) {
    std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

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

    *outSize = size;
    return buffer;
}

MonoAssembly* LoadCSharpAssembly(const std::string& assemblyPath) {
    if (!std::filesystem::exists(assemblyPath)) {
        std::cout << "path does not exist: " << assemblyPath << "\n";
        exit(1);
    }

    uint32_t fileSize = 0;
    char* fileData = ReadBytes(assemblyPath, &fileSize);

    // NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a
    // reference to the assembly
    MonoImageOpenStatus status;
    MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

    if (status != MONO_IMAGE_OK) {
        const char* errorMessage = mono_image_strerror(status);
        // Log some error message using the errorMessage data
        return nullptr;
    }

    MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
    mono_image_close(image);

    // Don't forget to free the file data
    delete[] fileData;

    return assembly;
}

// Create mono runtime, only needed to be called once during the lifetime of your app
bool InitialiseMono() {
    // You can choose what directory to load the core library .dll files from (mscorlib, System, System.Core)
    mono_set_assemblies_path("./mono/lib");

    // Initialise mono runtime
    rootDomain = mono_jit_init("YourRuntimeName");
    if (!rootDomain)
        return false; // Failed to initialise, log & return

    // Created successfully
    return true;
}

void CreateAppDomain() {
    // Create your app domain, this is what stores your loaded assemblies in memory.
    //  Second parameter is optional configuration file path
    appDomain = mono_domain_create_appdomain("YourCustomAppDomainName", nullptr);

    // Tell mono to set this domain as the default one.
    //   Second parameter forces this domain to be used
    mono_domain_set(appDomain, true);
}

int main() {
    // Setup scripting environment
    if (!InitialiseMono())
        return -1;

    CreateAppDomain();

    // Application loop
    // while(gameIsRunning) { }

    auto* assembly = LoadCSharpAssembly("../src/scripting/TestSolution/TestProject/bin/Debug/TestProject.dll");
    auto* image = mono_assembly_get_image(assembly);
    auto* c = mono_class_from_name(image, "TestProject", "ExampleClass");

    auto* class_instance = mono_object_new(appDomain, c);
    if (class_instance == nullptr)
        return 1;
    mono_runtime_object_init(class_instance);

    auto* floatField = mono_class_get_field_from_name(c, "MyPublicFloatVar");

    float* value;
    mono_field_get_value(class_instance, floatField, value);

    std::cout << *value << "\n";

    auto* method = mono_class_get_method_from_name(c, "IncrementFloatVar", 0);
    MonoObject* exception;
    mono_runtime_invoke(method, class_instance, nullptr, &exception);

    mono_field_get_value(class_instance, floatField, value);
    std::cout << *value << "\n";

    // Clean up scripting
    mono_jit_cleanup(rootDomain);
    rootDomain = nullptr;
    appDomain = nullptr;

    return 0;
}