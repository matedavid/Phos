#include "class_handle.h"

namespace Phos {

ClassHandle::ClassHandle(MonoClass* klass) : m_klass(klass) {
    MonoMethod* method;
    void* iter = nullptr;

    while ((method = mono_class_get_methods(m_klass, &iter))) {
        const std::string method_name = mono_method_get_name(method);
        m_methods.insert({method_name, method});
    }
}

ClassHandle::~ClassHandle() {
    for (const auto& [name, method] : m_methods) {
        mono_free_method(method);
    }
}

std::optional<MonoMethod*> ClassHandle::get_method(const std::string& name) const {
    const auto& iter = m_methods.find(name);
    if (iter != m_methods.end()) {
        return iter->second;
    }

    return {};
}

} // namespace Phos
