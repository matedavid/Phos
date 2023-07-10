#pragma once

#include "core.h"

namespace Phos {

class Skybox {
  public:
    struct Sides {
        std::string front;
        std::string back;
        std::string up;
        std::string down;
        std::string left;
        std::string right;
    };

    virtual ~Skybox() = default;

    static std::shared_ptr<Skybox> create(const Sides& sides);
    static std::shared_ptr<Skybox> create(const Sides& sides, const std::string& directory);
};

} // namespace Phos
