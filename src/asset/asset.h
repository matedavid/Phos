#pragma once

#include "core.h"

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
};

class IAsset {
  public:
    virtual AssetType asset_type() = 0;
    UUID id{};
};

} // namespace Phos
