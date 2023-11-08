#include "class_handle.h"

#include "scripting/scripting_engine.h"

namespace Phos {

// From mono/mono/metadata/tabledefs.h
#define FIELD_ATTRIBUTE_PUBLIC 0x0006

std::shared_ptr<ClassHandle> ClassHandle::create(const std::string& namespace_, const std::string& class_name) {
    const auto full_name = namespace_.empty() ? class_name : namespace_ + "." + class_name;
    if (ScriptingEngine::m_context.klass_cache.contains(full_name)) {
        return ScriptingEngine::m_context.klass_cache[class_name];
    }

    // @TODO: Maybe not the best idea??
    auto* image = ScriptingEngine::m_context.app_image;
    if (namespace_ == "PhosEngine")
        image = ScriptingEngine::m_context.core_image;

    auto* klass = mono_class_from_name(image, namespace_.data(), class_name.data());
    if (klass == nullptr) {
        PS_ERROR("No class found with name: '{}'", full_name);
        return nullptr;
    }

    const auto handle = std::shared_ptr<ClassHandle>(new ClassHandle(klass, full_name));
    ScriptingEngine::m_context.klass_cache[full_name] = handle;

    return handle;
}

ClassHandle::ClassHandle(MonoClass* klass, std::string name) : m_klass(klass), m_name(std::move(name)) {
    MonoMethod* method;
    void* iter = nullptr;

    while ((method = mono_class_get_methods(m_klass, &iter))) {
        const std::string method_name = mono_method_get_name(method);

        auto* signature = mono_method_signature(method);
        const auto num_params = mono_signature_get_param_count(signature);

        const auto method_id = MethodId(method_name, num_params);
        m_methods.insert({method_id, method});
    }

    MonoClassField* field;
    iter = nullptr;

    while ((field = mono_class_get_fields(m_klass, &iter))) {
        const std::string field_name = mono_field_get_name(field);
        auto* field_type = mono_field_get_type(field);

        if (field_name.starts_with('<'))
            continue;

        const auto flags = mono_field_get_flags(field);
        if (!(flags & FIELD_ATTRIBUTE_PUBLIC))
            continue;

        ClassFieldInfo::Type internal_type;
        switch (mono_type_get_type(field_type)) {
        case MONO_TYPE_I2:
        case MONO_TYPE_I4:
        case MONO_TYPE_I8:
        case MONO_TYPE_U2:
        case MONO_TYPE_U4:
        case MONO_TYPE_U8:
            internal_type = ClassFieldInfo::Type::Int;
            break;
        case MONO_TYPE_R4:
            fmt::print("type float\n");
            internal_type = ClassFieldInfo::Type::Float;
            break;
        case MONO_TYPE_STRING:
            internal_type = ClassFieldInfo::Type::String;
            break;
        case MONO_TYPE_CLASS: {
            const std::string type_name = mono_field_get_name(field);
            if (type_name == "PhosEngine.Entity")
                internal_type = ClassFieldInfo::Type::Entity;
            else if (type_name == "PhosEngine.Prefab")
                internal_type = ClassFieldInfo::Type::Prefab;
            else
                continue;
        } break;
        }

        const auto field_info = ClassFieldInfo{
            .name = field_name,
            .field = field,
            .field_type = internal_type,
        };
        m_fields.insert({field_name, field_info});
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

std::optional<ClassFieldInfo> ClassHandle::get_field(const std::string& name) const {
    const auto& iter = m_fields.find(name);
    if (iter != m_fields.end()) {
        return iter->second;
    }

    return {};
}

std::vector<ClassFieldInfo> ClassHandle::get_all_fields() const {
    // @TODO: Probably can optimize

    std::vector<ClassFieldInfo> fields;
    fields.reserve(m_fields.size());

    for (const auto& [name, info] : m_fields) {
        fields.push_back(info);
    }

    return fields;
}

} // namespace Phos
