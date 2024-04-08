#include "asset.h"

namespace Phos {

std::optional<std::string> AssetType::to_string(const AssetType& type) {
    switch (type) {
    case AssetType::Texture:
        return "texture";
    case AssetType::Shader:
        return "shader";
    case AssetType::Material:
        return "material";
    case AssetType::StaticMesh:
        return "StaticMesh";
    case AssetType::Scene:
        return "scene";
    case AssetType::Script:
        return "script";
    case AssetType::Cubemap:
        return "cubemap";
    case AssetType::Prefab:
        return "prefab";
    }

    return {};
}

std::optional<AssetType> AssetType::from_string(const std::string& str) {
    if (str == "texture")
        return AssetType::Texture;
    else if (str == "cubemap")
        return AssetType::Cubemap;
    else if (str == "shader")
        return AssetType::Shader;
    else if (str == "material")
        return AssetType::Material;
    else if (str == "StaticMesh")
        return AssetType::StaticMesh;
    else if (str == "prefab")
        return AssetType::Prefab;
    else if (str == "scene")
        return AssetType::Scene;
    else if (str == "script")
        return AssetType::Script;

    return {};
}

} // namespace Phos