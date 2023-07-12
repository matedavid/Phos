#pragma once

#include "core.h"

namespace Phos {

class Cubemap {
  public:
    struct Faces {
        std::string right;
        std::string left;
        std::string top;
        std::string bottom;
        std::string front;
        std::string back;
    };

    virtual ~Cubemap() = default;

    static std::shared_ptr<Cubemap> create(const Faces& faces);
    static std::shared_ptr<Cubemap> create(const Faces& faces, const std::string& directory);
};

} // namespace Phos
