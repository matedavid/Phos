#pragma once

#include "core.h"

#include <string>

namespace Phos {

enum class AssetType {
    Texture,
    Cubemap,
    Shader,
    Material,
};

class IAsset {
  public:
    virtual AssetType asset_type() = 0;
};

} // namespace Phos
