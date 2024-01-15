#pragma once

#include <string>
#include "core/uuid.h"

namespace Phos {

enum class AssetType {
    Texture,
    Cubemap,
    Shader,
    Material,
    Mesh,
    Model,
    Prefab,
    Scene,
    Script,
};

class IAsset {
  public:
    virtual ~IAsset() = default;

    virtual AssetType asset_type() = 0;
    UUID id{};
    std::string asset_name;
};

} // namespace Phos
