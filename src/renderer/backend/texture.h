#pragma once

#include "core.h"

namespace Phos {

// Forward declarations
class Image;

class Texture {
  public:
    virtual ~Texture() = default;

    static std::shared_ptr<Texture> create(const std::string& path);
    static std::shared_ptr<Texture> create(uint32_t width, uint32_t height);

    [[nodiscard]] virtual std::shared_ptr<Image> get_image() const = 0;
};

} // namespace Phos
