#pragma once

#include "core.h"

#include <optional>
#include <unordered_map>

#include <mono/jit/jit.h>

#include "asset/asset.h"
#include "scripting/class_field.h"

namespace Phos {

class ClassHandle : public IAsset {
  public:
    virtual ~ClassHandle();

    static std::shared_ptr<ClassHandle> create(const std::string& namespace_, const std::string& class_name);

    [[nodiscard]] AssetType asset_type() override { return AssetType::Script; }

    [[nodiscard]] std::optional<MonoMethod*> get_method(const std::string& name, uint32_t num_params = 0) const;
    [[nodiscard]] std::optional<ClassField> get_field(const std::string& name) const;

    [[nodiscard]] std::vector<ClassField> get_all_fields() const;

    [[nodiscard]] std::string class_name() const { return m_name; }
    [[nodiscard]] MonoClass* handle() const { return m_klass; }

  private:
    MonoClass* m_klass;
    std::string m_name;

    using MethodId = std::pair<std::string, uint32_t>;
    struct MethodId_Hash {
        std::size_t operator()(const MethodId& pair) const {
            std::size_t seed = 0;

            constexpr std::hash<std::string> hasher_t1;
            seed ^= hasher_t1(pair.first) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

            constexpr std::hash<uint32_t> hasher_t2;
            seed ^= hasher_t2(pair.second) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

            return seed;
        }
    };

    std::unordered_map<MethodId, MonoMethod*, MethodId_Hash> m_methods;

    std::unordered_map<std::string, ClassField> m_fields;

    ClassHandle(MonoClass* klass, std::string name);
};

} // namespace Phos
