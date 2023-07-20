#include "asset_loader.h"

#include <yaml-cpp/yaml.h>
#include <ranges>
#include <filesystem>

#include "managers/shader_manager.h"

#include "asset/asset_manager.h"

#include "renderer/backend/renderer.h"

#include "renderer/backend/texture.h"
#include "renderer/backend/cubemap.h"
#include "renderer/backend/material.h"

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

#define REGISTER_PARSER(Parser) m_parsers.push_back(std::make_unique<Parser>(m_manager))

AssetLoader::AssetLoader(AssetManager* manager) : m_manager(manager) {
    // Register parsers
    REGISTER_PARSER(TextureParser);
    REGISTER_PARSER(CubemapParser);
    REGISTER_PARSER(MaterialParser);
}

UUID AssetLoader::get_id(const std::string& path) const {
    const YAML::Node node = YAML::LoadFile(path);
    return UUID(node["id"].as<uint64_t>());
}

std::shared_ptr<IAsset> AssetLoader::load(const std::string& path) const {
    const YAML::Node node = YAML::LoadFile(path);

    const auto type_str = node["assetType"].as<std::string>();
    const AssetType type = string_to_asset_type(type_str);

    const auto id = UUID(node["id"].as<uint64_t>());

    const auto it = std::ranges::find_if(m_parsers, [&type](const auto& parser) { return parser->type() == type; });
    PS_ASSERT(it != m_parsers.end(), "Parser not found for asset type: {}\n", type_str)

    auto asset = (*it)->parse(node, path);
    asset->id = id;

    return asset;
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

//
// MaterialParser
//

std::shared_ptr<IAsset> MaterialParser::parse(const YAML::Node& node, [[maybe_unused]] const std::string& path) {
    const auto name = node["name"].as<std::string>();

    auto material = Material::create(Renderer::shader_manager()->get_builtin_shader("PBR.Geometry.Deferred"), name);

    const auto properties_node = node["properties"];
    for (const auto it : properties_node) {
        const auto property_name = it.first.as<std::string>();

        const auto property_type = properties_node[it.first]["type"].as<std::string>();
        const auto data_node = properties_node[it.first]["data"];

        if (property_type == "texture") {
            const auto texture = parse_texture(data_node);
            material->set(property_name, texture);
        } else if (property_type == "vec3") {
            const auto data = parse_vec3(data_node);
            material->set(property_name, data);
        } else if (property_type == "vec4") {
            const auto data = parse_vec4(data_node);
            material->set(property_name, data);
        } else {
            PS_WARNING("Property type '{}' is not valid", property_type);
        }
    }

    return material;
}

std::shared_ptr<Texture> MaterialParser::parse_texture(const YAML::Node& node) const {
    const auto id = UUID(node.as<uint64_t>());
    return m_manager->load_by_id<Texture>(id);
}

glm::vec3 MaterialParser::parse_vec3(const YAML::Node& node) const {
    const auto data_str = node.as<std::string>();
    const auto data = split_string(data_str);

    // TODO: Should check size of vector is correct

    return {data[0], data[1], data[2]};
}

glm::vec4 MaterialParser::parse_vec4(const YAML::Node& node) const {
    const auto data_str = node.as<std::string>();
    const auto data = split_string(data_str);

    // TODO: Should check size of vector is correct

    return {data[0], data[1], data[2], data[3]};
}

std::vector<float> MaterialParser::split_string(const std::string& str) const {
    std::vector<float> data;

    std::stringstream ss(str);
    std::string value;
    while (ss >> value)
        data.push_back(std::stof(value));

    return data;
}

} // namespace Phos
