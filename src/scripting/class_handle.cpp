#include "class_handle.h"

namespace Phos {

ClassHandle::ClassHandle(MonoClass* klass) : m_klass(klass) {
    MonoMethod* method;
    void* iter = nullptr;

    while ((method = mono_class_get_methods(m_klass, &iter))) {
        const std::string method_name = mono_method_get_name(method);

        auto* signature = mono_method_signature(method);
        const auto num_params = mono_signature_get_param_count(signature);

        const auto method_id = MethodId(method_name, num_params);
        m_methods.insert({method_id, method});
    }
}

ClassHandle::~ClassHandle() {
    for (const auto& [name, method] : m_methods) {
        mono_free_method(method);
    }
}

std::optional<MonoMethod*> ClassHandle::get_method(const std::string& name, uint32_t num_params) const {
    const auto method_id = MethodId(name, num_params);

    const auto& iter = m_methods.find(method_id);
    if (iter != m_methods.end()) {
        return iter->second;
    }

    return {};
}

} // namespace Phos
