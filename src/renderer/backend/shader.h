#pragma once

#include "core.h"

namespace Phos {

class Shader {
  public:
    virtual ~Shader() = default;

    static std::shared_ptr<Shader> create(const std::string& vertex_path, const std::string& fragment_path);
    static std::shared_ptr<Shader> create(const std::string& path);
};

} // namespace Phos
