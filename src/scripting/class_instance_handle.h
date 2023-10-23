#pragma once

#include "core.h"

#include <mono/jit/jit.h>

namespace Phos {

// Forward declarations
class ClassHandle;

class ClassInstanceHandle {
  public:
    ClassInstanceHandle(MonoObject* instance, std::shared_ptr<ClassHandle> class_handle);
    ~ClassInstanceHandle() = default;

    void invoke_on_create();
    void invoke_on_update();

  private:
    MonoObject* m_instance;
    std::shared_ptr<ClassHandle> m_class_handle;

    MonoMethod* m_on_create_method;
    MonoMethod* m_on_update_method;
};

} // namespace Phos
