#pragma once

#include "core.h"

namespace Phos {

class Image {
  public:
    enum class Type {
        Image2D,
        Image3D
    };

    enum class Format {
        B8G8R8_SRGB,
        D32_SFLOAT
    };

    struct Description {
        uint32_t width{};
        uint32_t height{};
        Type type = Type::Image2D;
        Format format = Format::B8G8R8_SRGB;

        bool transfer = false;   // Will the image be used for transfer operations
        bool attachment = false; // Will the image be used as an attachment of a Framebuffer
    };

    virtual ~Image() = default;

    static std::shared_ptr<Image> create(const Description& description);

    [[nodiscard]] virtual uint32_t width() const = 0;
    [[nodiscard]] virtual uint32_t height() const = 0;
    [[nodiscard]] virtual Format format() const = 0;
};

} // namespace Phos