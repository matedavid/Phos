#pragma once

#include <string>
#include <variant>
#include <mono/jit/jit.h>
#include <glm/glm.hpp>

#include "core/uuid.h"

namespace Phos {

struct PrefabRef {
    UUID id;
};

struct EntityRef {
    UUID id;
};

class ClassField {
  public:
    using Value = std::variant<int32_t, float, glm::vec3, std::string, PrefabRef, EntityRef>;

    enum class Type {
        Int,
        Float,
        Vec3,
        String,
        Entity,
        Prefab,
    };

    [[nodiscard]] static Value get_default_value(const Type& type);

    std::string name;
    MonoClassField* field;
    Type field_type;
};

} // namespace Phos
