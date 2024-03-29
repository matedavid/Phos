#pragma once

#include <mono/jit/jit.h>
#include "utility/logging.h"
#include "scripting/class_handle.h"

namespace Phos {

class ClassInstanceHandle {
  public:
    [[nodiscard]] static std::shared_ptr<ClassInstanceHandle> instantiate(std::shared_ptr<ClassHandle> class_handle);
    ~ClassInstanceHandle() = default;

    void invoke_constructor(const UUID& entity_id);
    void invoke_on_create();
    void invoke_on_update(double delta_time);

    template <typename T>
    void set_field_value(const std::string& name, const T& value) {
        auto info = m_class_handle->get_field(name);
        if (!info.has_value()) {
            PHOS_LOG_ERROR("Class {} does not have field: {} ", m_class_handle->class_name(), name);
            return;
        }

        set_field_value_internal(*info, (T*)&value);
    }

  private:
    MonoObject* m_instance = nullptr;
    std::shared_ptr<ClassHandle> m_class_handle;

    bool m_constructed = false;

    MonoMethod* m_constructor;
    MonoMethod* m_on_create_method;
    MonoMethod* m_on_update_method;

    explicit ClassInstanceHandle(std::shared_ptr<ClassHandle> class_handle);

    template <typename T>
    void set_field_value_internal(const ClassField& info, T* value);
};

} // namespace Phos
