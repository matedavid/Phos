#include "class_field.h"

namespace Phos {

ClassField::Value ClassField::get_default_value(const Type& type) {
    auto v = Value();

    switch (type) {
    case Type::Int:
        v = 0;
        break;
    case Type::Float:
        v = 0.0f;
        break;
    case Type::Vec3:
        v = glm::vec3();
        break;
    case Type::String:
        v = std::string();
        break;
    case Type::Prefab:
        v = PrefabRef{.id = Phos::UUID(0)};
        break;
    case Type::Entity:
        v = EntityRef{.id = Phos::UUID(0)};
        break;
    }

    return v;
}

} // namespace Phos
