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

#include "asset/prefab_asset.h"
#include "asset/asset_parsing_utils.h"
#include "asset/editor_asset_manager.h"

#include "renderer/backend/renderer.h"

#include "renderer/mesh.h"
#include "renderer/backend/texture.h"
#include "renderer/backend/cubemap.h"
#include "renderer/backend/material.h"

#include "scripting/class_instance_handle.h"

namespace Phos {

#define REGISTER_PARSER(Parser, Type) m_parsers.insert({AssetType::Type, std::make_unique<Parser>(m_manager)})

AssetLoader::AssetLoader(EditorAssetManager* manager) : m_manager(manager) {
    // Register parsers
    REGISTER_PARSER(TextureParser, Texture);
    REGISTER_PARSER(CubemapParser, Cubemap);
    REGISTER_PARSER(MaterialParser, Material);
    REGISTER_PARSER(StaticMeshParser, StaticMesh);
    REGISTER_PARSER(PrefabParser, Prefab);
    REGISTER_PARSER(SceneParser, Scene);
    REGISTER_PARSER(ScriptParser, Script);
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
    return *AssetType::from_string(node["assetType"].as<std::string>());
}

std::shared_ptr<IAsset> AssetLoader::load(const std::string& path) const {
    try {
        const YAML::Node node = YAML::LoadFile(path);

        const auto type_str = node["assetType"].as<std::string>();
        const AssetType type = *AssetType::from_string(type_str);

        const auto parser_it = m_parsers.find(type);
        if (parser_it == m_parsers.end()) {
            PHOS_LOG_WARNING("Parser not found for asset type: {}", type_str);
            return nullptr;
        }

        const auto id = UUID(node["id"].as<uint64_t>());

        auto asset = parser_it->second->parse(node, path);
        asset->id = id;
        asset->asset_name = std::filesystem::path(path).string();

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

    return Texture::create(texture_path.string());
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
            .right = load_face(right).string(),
            .left = load_face(left).string(),
            .top = load_face(top).string(),
            .bottom = load_face(bottom).string(),
            .front = load_face(front).string(),
            .back = load_face(back).string(),
        };
        return Cubemap::create(faces);
    }

    if (cubemap_type == "equirectangular") {
        const auto texture_id = UUID(node["texture"].as<uint64_t>());

        const auto texture_asset_path = m_manager->get_asset_path(texture_id);
        if (!texture_asset_path) {
            PHOS_LOG_WARNING("Could not find texture asset path with id {} for cubemap",
                             static_cast<uint64_t>(texture_id));
            return nullptr;
        }

        auto node_texture = YAML::LoadFile((m_manager->path() / *texture_asset_path).string());

        PHOS_ASSERT(node_texture["assetType"].as<std::string>() == AssetType::to_string(AssetType::Texture),
                    "Cubemap texture asset type is not texture ({})",
                    node_texture["assetType"].as<std::string>());

        const auto texture_complete_path = texture_asset_path->parent_path() / node_texture["path"].as<std::string>();
        return Cubemap::create(texture_complete_path.string());
    }

    PHOS_FAIL("Cubemap type '{}' not recognized", cubemap_type);
    return nullptr;
}

std::filesystem::path CubemapParser::load_face(const Phos::UUID& id) {
    const auto path = m_manager->get_asset_path(id);
    if (!path) {
        PHOS_LOG_WARNING("Could not find cubemap face with id {}", static_cast<uint64_t>(id));
        return {};
    }

    const auto containing_folder = m_manager->path() / path->parent_path();

    const auto node = YAML::LoadFile((m_manager->path() / *path).string());

    const auto asset_type = node["assetType"].as<std::string>();
    PHOS_ASSERT(asset_type == AssetType::to_string(AssetType::Texture),
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
// StaticMeshParser
//

std::shared_ptr<IAsset> StaticMeshParser::parse(const YAML::Node& node, [[maybe_unused]] const std::string& path) {
    const auto model_path = node["source"].as<std::string>();
    const auto real_model_path = std::filesystem::path(path).parent_path() / model_path;

    const uint32_t import_flags =
        aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(real_model_path.string().c_str(), import_flags);

    if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        PHOS_LOG_ERROR("Failed to parse StaticMesh, error loading file: {}\n", real_model_path.string());
        return nullptr;
    }

    // Load meshes
    PHOS_ASSERT(scene->mNumMeshes > 0, "StaticMesh must have at least one sub mesh");

    std::vector<std::shared_ptr<SubMesh>> meshes;
    for (std::size_t i = 0; i < scene->mNumMeshes; ++i) {
        const auto mesh = scene->mMeshes[i];

        std::vector<SubMesh::Vertex> vertices;
        std::vector<uint32_t> indices;

        // Vertices
        for (uint32_t j = 0; j < mesh->mNumVertices; ++j) {
            SubMesh::Vertex vertex{};

            // Position
            const auto& vs = mesh->mVertices[j];
            vertex.position.x = vs.x;
            vertex.position.y = vs.y;
            vertex.position.z = vs.z;

            // Normals
            if (mesh->HasNormals()) {
                const auto& ns = mesh->mNormals[j];
                vertex.normal.x = ns.x;
                vertex.normal.y = ns.y;
                vertex.normal.z = ns.z;
            }

            // Texture coordinates
            if (mesh->HasTextureCoords(0)) {
                const auto& tc = mesh->mTextureCoords[0][j];
                vertex.texture_coordinates.x = tc.x;
                vertex.texture_coordinates.y = 1.0f - tc.y;
            }

            // Tangents
            if (mesh->HasTangentsAndBitangents()) {
                const auto ts = mesh->mTangents[j];
                vertex.tangent.x = ts.x;
                vertex.tangent.y = ts.y;
                vertex.tangent.z = ts.z;
            }

            vertices.push_back(vertex);
        }

        // Indices
        for (uint32_t j = 0; j < mesh->mNumFaces; ++j) {
            const aiFace& face = mesh->mFaces[j];

            for (uint32_t idx = 0; idx < face.mNumIndices; ++idx) {
                indices.push_back(face.mIndices[idx]);
            }
        }

        meshes.push_back(std::make_shared<SubMesh>(vertices, indices));
    }

    return std::make_shared<StaticMesh>(meshes);
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
    return ClassHandle::create("", class_name.string());
}

} // namespace Phos
