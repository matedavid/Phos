#include "class_instance_handle.h"

namespace Phos {

ClassInstanceHandle::ClassInstanceHandle(MonoClass* klass, MonoObject* instance)
      : m_klass(klass), m_instance(instance) {
    m_on_create_method = mono_class_get_method_from_name(m_klass, "OnCreate", 0);
    m_on_update_method = mono_class_get_method_from_name(m_klass, "OnUpdate", 0);
}

ClassInstanceHandle::~ClassInstanceHandle() {
    mono_free_method(m_on_create_method);
    mono_free_method(m_on_update_method);
}

void ClassInstanceHandle::invoke_on_create() {
    MonoObject* exception;
    mono_runtime_invoke(m_on_create_method, m_instance, nullptr, &exception);
}

void ClassInstanceHandle::invoke_on_update() {
    MonoObject* exception;
    mono_runtime_invoke(m_on_update_method, m_instance, nullptr, &exception);

    // TEST
    auto* floatField = mono_class_get_field_from_name(m_klass, "MyPublicFloatVar");
    float* value;
    mono_field_get_value(m_instance, floatField, value);
    fmt::print("Value after ScriptEngine::on_update: {}\n", *value);
}

} // namespace Phos
