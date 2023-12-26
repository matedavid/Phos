#pragma once

#include <memory>

namespace Phos {

class Image {
  public:
    enum class Type {
        Image2D,
        Image3D,
        Cubemap
    };

    enum class Format {
        B8G8R8A8_SRGB,
        R8G8B8A8_SRGB,
        R8G8B8A8_UNORM,
        R16G16B16A16_SFLOAT,
        R32G32B32A32_SFLOAT,
        D32_SFLOAT
    };

    struct Description {
        uint32_t width{};
        uint32_t height{};
        Type type = Type::Image2D;
        Format format = Format::B8G8R8A8_SRGB;
        uint32_t num_layers = 1;
        bool generate_mips = false;

        bool transfer = false;   // Will the image be used for transfer operations
        bool attachment = false; // Will the image be used as an attachment of a Framebuffer
        bool storage = false;    // Will the image be used as storage parameters in compute shaders
    };

    virtual ~Image() = default;

    static std::shared_ptr<Image> create(const Description& description);

    [[nodiscard]] virtual uint32_t width() const = 0;
    [[nodiscard]] virtual uint32_t height() const = 0;
    [[nodiscard]] virtual Format format() const = 0;
    [[nodiscard]] virtual uint32_t num_mips() const = 0;
};

} // namespace Phos
