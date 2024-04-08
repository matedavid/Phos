#include "asset.h"

namespace Phos {

std::optional<std::string> AssetType::to_string(const AssetType& type) {
    switch (type) {
    case AssetType::Texture:
        return "Texture";
    case AssetType::Shader:
        return "Shader";
    case AssetType::Material:
        return "Material";
    case AssetType::StaticMesh:
        return "StaticMesh";
    case AssetType::Scene:
        return "Scene";
    case AssetType::Script:
        return "script";
    case AssetType::Cubemap:
        return "Cubemap";
    case AssetType::Prefab:
        return "Prefab";
    }

    return {};
}

std::optional<AssetType> AssetType::from_string(const std::string& str) {
    if (str == "Texture")
        return AssetType::Texture;
    else if (str == "Cubemap")
        return AssetType::Cubemap;
    else if (str == "Shader")
        return AssetType::Shader;
    else if (str == "Material")
        return AssetType::Material;
    else if (str == "StaticMesh")
        return AssetType::StaticMesh;
    else if (str == "Prefab")
        return AssetType::Prefab;
    else if (str == "Scene")
        return AssetType::Scene;
    else if (str == "Script")
        return AssetType::Script;

    return {};
}

} // namespace Phos