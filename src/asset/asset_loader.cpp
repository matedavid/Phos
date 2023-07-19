#include "asset_loader.h"

#include <yaml-cpp/yaml.h>
#include <ranges>
#include <filesystem>

#include "renderer/backend/texture.h"
#include "renderer/backend/cubemap.h"

namespace Phos {

static AssetType string_to_asset_type(const std::string& str) {
    if (str == "texture")
        return AssetType::Texture;
    else if (str == "cubemap")
        return AssetType::Cubemap;
    else if (str == "shader")
        return AssetType::Shader;
    else if (str == "material")
        return AssetType::Material;

    PS_FAIL("Not valid asset type")
}

#define REGISTER_PARSER(Parser) m_parsers.push_back(std::make_unique<Parser>())

AssetLoader::AssetLoader() {
    // Register parsers
    REGISTER_PARSER(TextureParser);
    REGISTER_PARSER(CubemapParser);
}

std::shared_ptr<IAsset> AssetLoader::load(const std::string& path) {
    const YAML::Node node = YAML::LoadFile(path);

    const auto type_str = node["assetType"].as<std::string>();
    const AssetType type = string_to_asset_type(type_str);

    const auto it = std::ranges::find_if(m_parsers, [&type](const auto& parser) { return parser->type() == type; });
    PS_ASSERT(it != m_parsers.end(), "Parser not found for asset type: {}\n", type_str)

    return (*it)->parse(node, path);
}

//
// TextureParser
//

std::shared_ptr<IAsset> TextureParser::parse(const YAML::Node& node, [[maybe_unused]] const std::string& path) {
    const auto containing_folder = std::filesystem::path(path).parent_path();
    const auto texture_path = containing_folder / node["path"].as<std::string>();

    return Texture::create(texture_path);
}

//
// CubemapParser
//

std::shared_ptr<IAsset> CubemapParser::parse(const YAML::Node& node, [[maybe_unused]] const std::string& path) {
    const auto containing_folder = std::filesystem::path(path).parent_path();

    const auto faces_node = node["faces"];

    const auto left = faces_node["left"].as<std::string>();
    const auto right = faces_node["right"].as<std::string>();
    const auto top = faces_node["top"].as<std::string>();
    const auto bottom = faces_node["bottom"].as<std::string>();
    const auto front = faces_node["front"].as<std::string>();
    const auto back = faces_node["back"].as<std::string>();

    const Cubemap::Faces faces = {
        .right = right,
        .left = left,
        .top = top,
        .bottom = bottom,
        .front = front,
        .back = back,
    };
    return Cubemap::create(faces, containing_folder);
}

} // namespace Phos
