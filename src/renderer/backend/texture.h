#pragma once

#include "core.h"

#include "asset/asset.h"

namespace Phos {

// Forward declarations
class Image;

class Texture : public IAsset {
  public:
    virtual ~Texture() = default;

    [[nodiscard]] AssetType asset_type() override { return AssetType::Texture; }

    static std::shared_ptr<Texture> create(const std::string& path);
    static std::shared_ptr<Texture> create(uint32_t width, uint32_t height);
    static std::shared_ptr<Texture> create(const std::vector<char>& data, uint32_t width, uint32_t height);
    static std::shared_ptr<Texture> create(const std::shared_ptr<Image>& image);

    [[nodiscard]] static std::shared_ptr<Texture> white(uint32_t width, uint32_t height);

    [[nodiscard]] virtual std::shared_ptr<Image> get_image() const = 0;
};

} // namespace Phos
