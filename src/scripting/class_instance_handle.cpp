#include "class_instance_handle.h"

#include "core/uuid.h"
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

    // @TODO: mono_field_set_value(m_instance, info.field, value);
}

SET_FIELD_VALUE_INTERNAL_FUNC(UUID) {
    check_type(info, ClassFieldInfo::Type::String, "entity, prefab");

    auto id = (uint64_t)value;
    mono_field_set_value(m_instance, info.field, &id);
}

} // namespace Phos
