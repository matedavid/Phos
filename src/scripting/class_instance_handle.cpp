#include "class_instance_handle.h"

#include "scripting/class_handle.h"

namespace Phos {

ClassInstanceHandle::ClassInstanceHandle(MonoObject* instance, std::shared_ptr<ClassHandle> class_handle)
      : m_instance(instance), m_class_handle(std::move(class_handle)) {
    m_on_create_method = *m_class_handle->get_method("OnCreate");
    m_on_update_method = *m_class_handle->get_method("OnUpdate", 1);
}

void ClassInstanceHandle::invoke_on_create() {
    MonoObject* exception;
    mono_runtime_invoke(m_on_create_method, m_instance, nullptr, &exception);
}

static void print_fields(MonoClass* klass) {
    MonoClassField* field;
    void* iter = nullptr;

    while ((field = mono_class_get_fields(klass, &iter))) {
        const char* field_name = mono_field_get_name(field);
        MonoType* field_type = mono_field_get_type(field);

        fmt::print("Field Name: {}\n", field_name);
        fmt::print("Field Type: {}\n", mono_type_get_name(field_type));
    }
}

void ClassInstanceHandle::invoke_on_update(double delta_time) {
    auto dt = static_cast<float>(delta_time); // ScriptGlue works with floats

    void* args[1];
    args[0] = &dt;

    MonoObject* exception;
    mono_runtime_invoke(m_on_update_method, m_instance, args, &exception);
}

} // namespace Phos
