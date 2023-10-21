#pragma once

#include "core.h"

#include <mono/jit/jit.h>

namespace Phos {

class ClassInstanceHandle {
  public:
    ClassInstanceHandle(MonoClass* klass, MonoObject* instance);
    ~ClassInstanceHandle();

    void invoke_on_create();
    void invoke_on_update();

  private:
    MonoClass* m_klass;
    MonoObject* m_instance;

    MonoMethod* m_on_create_method;
    MonoMethod* m_on_update_method;
};

} // namespace Phos
