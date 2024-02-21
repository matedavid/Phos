#include "asset_loader.h"

#include <yaml-cpp/yaml.h>
#include <ranges>
#include <filesystem>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "utility/logging.h"

#include "managers/shader_manager.h"
#include "managers/texture_manager.h"

#include "scene/entity_deserializer.h"

#include "asset/asset_manager.h"
#include "asset/model_asset.h"
#include "asset/prefab_asset.h"
#include "asset/asset_parsing_utils.h"

#include "renderer/backend/renderer.h"

#include "renderer/mesh.h"
#include "renderer/backend/texture.h"
#include "renderer/backend/cubemap.h"
#include "renderer/backend/material.h"

#include "scripting/class_instance_handle.h"

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
    else if (str == "mesh")
        return AssetType::Mesh;
    else if (str == "model")
        return AssetType::Model;
    else if (str == "prefab")
        return AssetType::Prefab;
    else if (str == "scene")
        return AssetType::Scene;
    else if (str == "script")
        return AssetType::Script;

    PHOS_FAIL("Asset type '{}' not recognized", str);
}

#define REGISTER_PARSER(Parser) m_parsers.push_back(std::make_unique<Parser>(m_manager))

AssetLoader::AssetLoader(AssetManagerBase* manager) : m_manager(manager) {
    // Register parsers
    REGISTER_PARSER(TextureParser);
    REGISTER_PARSER(CubemapParser);
    REGISTER_PARSER(MaterialParser);
    REGISTER_PARSER(MeshParser);
    REGISTER_PARSER(ModelParser);
    REGISTER_PARSER(PrefabParser);
    REGISTER_PARSER(SceneParser);
    REGISTER_PARSER(ScriptParser);
}

UUID AssetLoader::get_id(const std::string& path) const {
    try {
        const YAML::Node node = YAML::LoadFile(path);
        return UUID(node["id"].as<uint64_t>());
    } catch (const std::exception&) {
        PHOS_LOG_ERROR("Exception while getting id of asset with path: '{}'", path);
        return UUID(0);
    }
}

AssetType AssetLoader::get_type(const std::string& path) const {
    const YAML::Node node = YAML::LoadFile(path);
    return string_to_asset_type(node["assetType"].as<std::string>());
}

std::shared_ptr<IAsset> AssetLoader::load(const std::string& path) const {
    try {
        const YAML::Node node = YAML::LoadFile(path);

        const auto type_str = node["assetType"].as<std::string>();
        const AssetType type = string_to_asset_type(type_str);

        const auto id = UUID(node["id"].as<uint64_t>());

        const auto it = std::ranges::find_if(m_parsers, [&type](const auto& parser) { return parser->type() == type; });
        PHOS_ASSERT(it != m_parsers.end(), "Parser not found for asset type: {}\n", type_str);

        auto asset = (*it)->parse(node, path);
        asset->id = id;
        asset->asset_name = std::filesystem::path(path).filename();

        return asset;
    } catch (const std::exception&) {
        PHOS_LOG_ERROR("Exception while loading asset with path: '{}'", path);
        return nullptr;
    }
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
    const auto cubemap_type = node["type"].as<std::string>();
    if (cubemap_type == "faces") {
        const auto faces_node = node["faces"];

        const auto left = Phos::UUID(faces_node["left"].as<uint64_t>());
        const auto right = Phos::UUID(faces_node["right"].as<uint64_t>());
        const auto top = Phos::UUID(faces_node["top"].as<uint64_t>());
        const auto bottom = Phos::UUID(faces_node["bottom"].as<uint64_t>());
        const auto front = Phos::UUID(faces_node["front"].as<uint64_t>());
        const auto back = Phos::UUID(faces_node["back"].as<uint64_t>());

        // @TODO: Supposing all face fields have valid value

        const Cubemap::Faces faces = {
            .right = load_face(right),
            .left = load_face(left),
            .top = load_face(top),
            .bottom = load_face(bottom),
            .front = load_face(front),
            .back = load_face(back),
        };
        return Cubemap::create(faces);
    }

    if (cubemap_type == "equirectangular") {
        const auto texture_id = UUID(node["texture"].as<uint64_t>());

        const auto texture_asset_path = m_manager->get_asset_path(texture_id);
        auto node_texture = YAML::LoadFile(texture_asset_path);

        PHOS_ASSERT(node_texture["assetType"].as<std::string>() == "texture",
                    "Cubemap texture asset type is not texture ({})",
                    node_texture["assetType"].as<std::string>());

        const auto texture_complete_path = texture_asset_path.parent_path() / node_texture["path"].as<std::string>();
        return Cubemap::create(texture_complete_path);
    }

    PHOS_FAIL("Cubemap type '{}' not recognized", cubemap_type);
    return nullptr;
}

std::filesystem::path CubemapParser::load_face(const Phos::UUID& id) {
    const auto path = m_manager->get_asset_path(id);
    const auto containing_folder = path.parent_path();

    const auto node = YAML::LoadFile(path);

    const auto asset_type = node["assetType"].as<std::string>();
    PHOS_ASSERT(asset_type == "texture",
                "Cubemap face with id {} is not of type texture ({})",
                static_cast<uint64_t>(id),
                asset_type);

    const auto face_local_path = node["path"].as<std::string>();
    return containing_folder / face_local_path;
}

//
// MaterialParser
//

std::shared_ptr<IAsset> MaterialParser::parse(const YAML::Node& node, [[maybe_unused]] const std::string& path) {
    const auto material_name = node["name"].as<std::string>();

    std::shared_ptr<Material> material;

    const auto shader_node = node["shader"];
    if (shader_node["type"].as<std::string>() == "builtin") {
        const auto shader_name = shader_node["name"].as<std::string>();
        material = Material::create(Renderer::shader_manager()->get_builtin_shader(shader_name), material_name);
    } else {
        PHOS_FAIL("At the moment, only builtin shaders are supported");
        return nullptr;
    }

    const auto properties_node = node["properties"];
    for (const auto it : properties_node) {
        const auto property_name = it.first.as<std::string>();

        const auto property_type = properties_node[it.first]["type"].as<std::string>();
        const auto data_node = properties_node[it.first]["data"];

        if (property_type == "texture") {
            const auto texture = parse_texture(data_node);
            material->set(property_name, texture);
        } else if (property_type == "vec3") {
            const auto data = AssetParsingUtils::parse_vec3(data_node);
            material->set(property_name, data);
        } else if (property_type == "vec4") {
            const auto data = AssetParsingUtils::parse_vec4(data_node);
            material->set(property_name, data);
        } else if (property_type == "float") {
            const auto data = AssetParsingUtils::parse_numeric<float>(data_node);
            material->set(property_name, data);
        } else {
            PHOS_LOG_WARNING("Property type '{}' is not valid", property_type);
        }
    }

    [[maybe_unused]] const auto baked = material->bake();
    PHOS_ASSERT(baked, "Could not bake material");

    return material;
}

std::shared_ptr<Texture> MaterialParser::parse_texture(const YAML::Node& node) const {
    const auto id = UUID(node.as<uint64_t>());
    if (id == UUID(0))
        return Renderer::texture_manager()->get_white_texture();

    return m_manager->load_by_id_type<Texture>(id);
}

//
// MeshParser
//

std::shared_ptr<IAsset> MeshParser::parse(const YAML::Node& node, [[maybe_unused]] const std::string& path) {
    const auto model_path = node["path"].as<std::string>();
    const auto index = node["index"].as<uint32_t>();

    const auto containing_folder = std::filesystem::path(path).parent_path();
    const auto real_model_path = containing_folder / model_path;

    constexpr uint32_t import_flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(real_model_path.c_str(), import_flags);

    if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        PHOS_LOG_ERROR("Failed to open file: {}\n", real_model_path.string());
        return nullptr;
    }

    const auto mesh = scene->mMeshes[index];

    std::vector<Mesh::Vertex> vertices;
    std::vector<uint32_t> indices;

    // Vertices
    for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
        Mesh::Vertex vertex{};

        // Position
        const auto& vs = mesh->mVertices[i];
        vertex.position.x = vs.x;
        vertex.position.y = vs.y;
        vertex.position.z = vs.z;

        // Normals
        if (mesh->HasNormals()) {
            const auto& ns = mesh->mNormals[i];
            vertex.normal.x = ns.x;
            vertex.normal.y = ns.y;
            vertex.normal.z = ns.z;
        }

        // Texture coordinates
        if (mesh->HasTextureCoords(0)) {
            const auto& tc = mesh->mTextureCoords[0][i];
            vertex.texture_coordinates.x = tc.x;
            vertex.texture_coordinates.y = 1.0f - tc.y;
        }

        // Tangents
        if (mesh->HasTangentsAndBitangents()) {
            const auto ts = mesh->mTangents[i];
            vertex.tangent.x = ts.x;
            vertex.tangent.y = ts.y;
            vertex.tangent.z = ts.z;
        }

        vertices.push_back(vertex);
    }

    // Indices
    for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
        const aiFace& face = mesh->mFaces[i];

        for (uint32_t idx = 0; idx < face.mNumIndices; ++idx) {
            indices.push_back(face.mIndices[idx]);
        }
    }

    return std::make_shared<Mesh>(vertices, indices);
}

//
// ModelParser
//

std::shared_ptr<IAsset> ModelParser::parse(const YAML::Node& node, [[maybe_unused]] const std::string& path) {
    const auto& parent_node = node["node_0"];
    auto* node_parent = parse_node_r(parent_node);

    return std::make_shared<ModelAsset>(node_parent);
}

ModelAsset::Node* ModelParser::parse_node_r(const YAML::Node& node) const {
    auto* n = new ModelAsset::Node();

    if (const auto mesh_node = node["mesh"]) {
        const auto mesh_id = UUID(mesh_node.as<uint64_t>());

        const auto mesh = m_manager->load_by_id_type<Mesh>(mesh_id);
        n->mesh = mesh;
    }

    if (const auto material_node = node["material"]) {
        const auto material_id = UUID(material_node.as<uint64_t>());

        const auto material = m_manager->load_by_id_type<Material>(material_id);
        n->material = material;
    }

    if (auto children_node = node["children"]) {
        for (auto child : children_node) {
            const auto child_node = children_node[child.first];

            auto* c = parse_node_r(child_node);
            n->children.push_back(c);
        }
    }

    return n;
}

//
// PrefabParser
//

std::shared_ptr<IAsset> PrefabParser::parse(const YAML::Node& node, [[maybe_unused]] const std::string& path) {
    // @TODO: Should probably rethink this approach of loading prefabs...
    std::stringstream ss;
    ss << node["components"];

    return std::make_shared<PrefabAsset>(ss.str());
}

//
// SceneParser
//

std::shared_ptr<IAsset> SceneParser::parse(const YAML::Node& node, [[maybe_unused]] const std::string& path) {
    const auto name = node["name"].as<std::string>();
    auto scene = std::make_shared<Scene>(name);

    // Load scene config
    const auto config_node = node["config"];
    auto& renderer_config = scene->config();

    renderer_config.rendering_config.shadow_map_resolution =
        config_node["renderingConfig"]["shadowMapResolution"].as<uint32_t>();

    renderer_config.bloom_config.enabled = config_node["bloomConfig"]["enabled"].as<bool>();
    renderer_config.bloom_config.threshold = config_node["bloomConfig"]["threshold"].as<float>();

    const auto skybox_id = UUID(config_node["environmentConfig"]["skybox"].as<uint64_t>());
    renderer_config.environment_config.skybox =
        skybox_id == UUID(0) ? nullptr : m_manager->load_by_id_type<Phos::Cubemap>(skybox_id);

    // Load entities
    const auto entities = node["entities"];
    for (const auto& it : entities) {
        const auto id = UUID(it.first.as<uint64_t>());
        EntityDeserializer::deserialize(entities[it.first], id, scene, m_manager);
    }

    return scene;
}

//
// ScriptParser
//

std::shared_ptr<IAsset> ScriptParser::parse([[maybe_unused]] const YAML::Node& node,
                                            [[maybe_unused]] const std::string& path) {
    // First stem to remove .psa and second to remove .cs
    const auto class_name = std::filesystem::path(path).stem().stem();
    return ClassHandle::create("", class_name);
}

} // namespace Phos
