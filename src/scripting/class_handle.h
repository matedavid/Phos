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

    [[nodiscard]] std::optional<MonoMethod*> get_method(const std::string& name, uint32_t num_params = 0) const;

    [[nodiscard]] MonoClass* handle() const { return m_klass; }

  private:
    MonoClass* m_klass;

    using MethodId = std::pair<std::string, uint32_t>;
    struct MethodId_Hash {
        std::size_t operator()(const MethodId& pair) const {
            std::size_t seed = 0;

            std::hash<std::string> hasher_t1;
            seed ^= hasher_t1(pair.first) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

            std::hash<uint32_t> hasher_t2;
            seed ^= hasher_t2(pair.second) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

            return seed;
        }
    };

    std::unordered_map<MethodId, MonoMethod*, MethodId_Hash> m_methods;
};

} // namespace Phos
