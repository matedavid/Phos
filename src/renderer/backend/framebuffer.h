#pragma once

#include "core.h"

#include <glm/glm.hpp>

namespace Phos {

// Forward declarations
class Image;

enum class LoadOperation {
    Load,
    Clear,
    DontCare
};

enum class StoreOperation {
    Store,
    DontCare
};

class Framebuffer {
  public:
    struct Attachment {
        std::shared_ptr<Image> image;

        LoadOperation load_operation;
        StoreOperation store_operation;

        glm::vec3 clear_value;

        bool is_presentation = false; // If attachment image will be used for presentation

        // If the result of the depth attachment, after the render pass finishes,
        // will be used as an input for another graphics pipeline or render pass
        bool input_depth = false;
    };

    struct Description {
        std::vector<Attachment> attachments;
    };

    virtual ~Framebuffer() = default;

    static std::shared_ptr<Framebuffer> create(const Description& description);

    [[nodiscard]] virtual uint32_t width() const = 0;
    [[nodiscard]] virtual uint32_t height() const = 0;
    [[nodiscard]] virtual const std::vector<Attachment>& get_attachments() const = 0;
};

} // namespace Phos