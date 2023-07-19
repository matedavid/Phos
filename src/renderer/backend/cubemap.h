#pragma once

#include "core.h"

#include "asset/asset.h"

namespace Phos {

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
};

} // namespace Phos
