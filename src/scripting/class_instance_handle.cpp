#include "class_instance_handle.h"

#include "core/uuid.h"

#include "scripting/class_handle.h"
#include "scripting/scripting_engine.h"

namespace Phos {

std::shared_ptr<ClassInstanceHandle> ClassInstanceHandle::instantiate(std::shared_ptr<ClassHandle> class_handle) {
    return std::shared_ptr<ClassInstanceHandle>(new ClassInstanceHandle(std::move(class_handle)));
}

ClassInstanceHandle::ClassInstanceHandle(std::shared_ptr<ClassHandle> class_handle)
      : m_class_handle(std::move(class_handle)) {
    m_instance = mono_object_new(ScriptingEngine::m_context.app_domain, m_class_handle->handle());
    if (m_instance == nullptr) {
        PS_ERROR("Failed to create class instance for class: {}", m_class_handle->class_name());
    }

    m_constructor = *ScriptingEngine::m_context.entity_class_handle->get_method(".ctor", 1);
    m_on_create_method = *m_class_handle->get_method("OnCreate");
    m_on_update_method = *m_class_handle->get_method("OnUpdate", 1);
}

void ClassInstanceHandle::invoke_constructor(const UUID& entity_id) {
    PS_ASSERT(!m_constructed,
              "Class's instance for class '{}' constructor has already been called",
              m_class_handle->class_name())

    void* args[1];
    auto id = (uint64_t)entity_id;
    args[0] = &id;

    mono_runtime_invoke(m_constructor, m_instance, args, nullptr);
    m_constructed = true;
}

void ClassInstanceHandle::invoke_on_create() {
    MonoObject* exception;
    mono_runtime_invoke(m_on_create_method, m_instance, nullptr, &exception);
}

void ClassInstanceHandle::invoke_on_update(double delta_time) {
    auto dt = static_cast<float>(delta_time); // ScriptGlue works with floats

    void* args[1];
    args[0] = &dt;

    MonoObject* exception;
    mono_runtime_invoke(m_on_update_method, m_instance, args, &exception);
}

//
// set_field_value_internal
//

#define SET_FIELD_VALUE_INTERNAL_FUNC(T) \
    template <>                          \
    void ClassInstanceHandle::set_field_value_internal<T>(const ClassFieldInfo& info, T* value)

void check_type(const ClassFieldInfo& info, ClassFieldInfo::Type expected, std::string_view actual_name) {
    PS_ASSERT(info.field_type == expected,
              "Trying to set field '{}' value with invalid type, provided type was: '{}'",
              info.name,
              actual_name);
}

SET_FIELD_VALUE_INTERNAL_FUNC(int32_t) {
    check_type(info, ClassFieldInfo::Type::Int, "int");

    mono_field_set_value(m_instance, info.field, value);
}

SET_FIELD_VALUE_INTERNAL_FUNC(float) {
    check_type(info, ClassFieldInfo::Type::Float, "float");

    mono_field_set_value(m_instance, info.field, value);
}

SET_FIELD_VALUE_INTERNAL_FUNC(std::string) {
    check_type(info, ClassFieldInfo::Type::String, "string");

    auto* mono_string = mono_string_new(ScriptingEngine::m_context.app_domain, value->c_str());
    mono_field_set_value(m_instance, info.field, mono_string);

    mono_free(mono_string);
}

SET_FIELD_VALUE_INTERNAL_FUNC(UUID) {
    check_type(info, ClassFieldInfo::Type::String, "entity, prefab");

    auto id = (uint64_t)value;
    mono_field_set_value(m_instance, info.field, &id);
}

} // namespace Phos
