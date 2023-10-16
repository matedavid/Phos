#pragma once

#include "core.h"

#include "asset/asset.h"

namespace Phos {

// Forward declarations
class Image;

class Cubemap : public IAsset {
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

    [[nodiscard]] AssetType asset_type() override { return AssetType::Cubemap; }

    static std::shared_ptr<Cubemap> create(const Faces& faces);
    static std::shared_ptr<Cubemap> create(const Faces& faces, const std::string& directory);
    static std::shared_ptr<Cubemap> create(const std::string& equirectangular_path);

    [[nodiscard]] virtual std::shared_ptr<Image> get_image() const = 0;
};

} // namespace Phos
