#pragma once

#include "core.h"

#include <optional>
#include <unordered_map>

#include <mono/jit/jit.h>

namespace Phos {

class ClassHandle {
  public:
    explicit ClassHandle(MonoClass* klass);
    ~ClassHandle();

    [[nodiscard]] std::optional<MonoMethod*> get_method(const std::string& name) const;

    [[nodiscard]] MonoClass* handle() const { return m_klass; }

  private:
    MonoClass* m_klass;

    std::unordered_map<std::string, MonoMethod*> m_methods;
};

} // namespace Phos
