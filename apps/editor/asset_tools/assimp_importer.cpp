#include "assimp_importer.h"

#include <fstream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "utility/logging.h"

#include "asset_tools/asset_builder.h"
#include "asset_tools/asset_importer.h"

#include "editor_material_helper.h"

#include "managers/shader_manager.h"
#include "renderer/backend/renderer.h"

// constexpr uint32_t import_flags =
//     aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph;

constexpr uint32_t import_flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes;

std::filesystem::path AssimpImporter::import_model(const std::filesystem::path& path,
                                                   const std::filesystem::path& containing_folder) {
    PHOS_ASSERT(std::filesystem::exists(path), "Model path: '{}' does not exist", path.string());

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path.c_str(), import_flags);

    if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        PHOS_LOG_ERROR("[AssimpImporter::import_model] Failed to load model: '{}'", path.string());
        return {};
    }

    const auto output_folder_path = containing_folder / scene->mName.C_Str();
    if (!std::filesystem::exists(output_folder_path))
        std::filesystem::create_directory(output_folder_path);

    PHOS_LOG_INFO("Loading model: '{}'", path.filename().string());
    PHOS_LOG_INFO("     Num meshes: {}", scene->mNumMeshes);
    PHOS_LOG_INFO("     Num materials: {}", scene->mNumMaterials);

    // Copy original asset to output_folder_path
    if (const auto output_model_path = output_folder_path / path.filename();
        !std::filesystem::exists(output_model_path)) {
        std::filesystem::copy(path, output_model_path);
    } else {
        PHOS_LOG_WARNING("[AssimpImporter::import_model] Model file in project ({}) already exists",
                         output_model_path.string());
    }

    if (path.extension() == ".gltf") {
        const auto bin_path = path.parent_path() / (path.stem().string() + ".bin");
        const auto bin_output_path = output_folder_path / bin_path.filename();
        if (!std::filesystem::exists(bin_output_path)) {
            std::filesystem::copy(bin_path, bin_output_path);
        } else {
            PHOS_LOG_WARNING("[AssimpImporter::import_model] GLTF bin file in project ({}) already exists",
                             bin_path.string());
        }
    }

    // Load meshes
    std::vector<Phos::UUID> mesh_ids;
    for (std::size_t i = 0; i < scene->mNumMeshes; ++i) {
        const auto id = load_mesh(i, path.filename(), output_folder_path);
        mesh_ids.push_back(id);
    }

    // Load textures
    std::unordered_map<std::string, Phos::UUID> texture_map;
    for (std::size_t i = 0; i < scene->mNumTextures; ++i) {
        const auto texture = scene->mTextures[i];
        const auto texture_path = path.parent_path() / texture->mFilename.C_Str();
        const auto new_texture_path = containing_folder / std::filesystem::path(texture->mFilename.C_Str()).filename();

        if (std::filesystem::exists(new_texture_path))
            continue;

        texture_map[texture->mFilename.C_Str()] = load_texture(texture_path, new_texture_path);
    }

    // Load materials
    std::vector<Phos::UUID> material_ids;
    for (std::size_t i = 0; i < scene->mNumMaterials; ++i) {
        const auto id = load_material(scene->mMaterials[i], i, texture_map, path.parent_path(), output_folder_path);
        material_ids.push_back(id);
    }

    // Process scene
    auto model_builder = AssetBuilder();

    model_builder.dump("assetType", "model");
    model_builder.dump("id", Phos::UUID());

    const auto root_node_builder = process_model_node_r(scene->mRootNode, scene, mesh_ids, material_ids);
    model_builder.dump("node_0", root_node_builder);

    const auto model_psa_path = output_folder_path / (path.filename().string() + ".psa");
    std::ofstream file(model_psa_path);
    file << model_builder;

    return output_folder_path;
}

Phos::UUID AssimpImporter::load_mesh(std::size_t idx,
                                     const std::filesystem::path& model_path,
                                     const std::filesystem::path& output_path) {
    const auto asset_id = Phos::UUID();

    auto mesh_builder = AssetBuilder();

    mesh_builder.dump("assetType", "mesh");
    mesh_builder.dump("id", asset_id);

    mesh_builder.dump("path", model_path);
    mesh_builder.dump("index", idx);

    const auto path = output_path / ("mesh_" + std::to_string(idx) + ".psa");
    std::ofstream file(path);
    file << mesh_builder;

    return asset_id;
}

Phos::UUID AssimpImporter::load_material(const aiMaterial* mat,
                                         std::size_t idx,
                                         std::unordered_map<std::string, Phos::UUID>& texture_map,
                                         const std::filesystem::path& parent_model_path,
                                         const std::filesystem::path& output_path) {
    const auto shader = Phos::Renderer::shader_manager()->get_builtin_shader("PBR.Geometry.Deferred");

    const std::string mat_name = mat->GetName().C_Str();
    const auto material = EditorMaterialHelper::create(shader, mat_name);

    const auto get_texture_if_exists_else_add = [&](const std::string& name) -> Phos::UUID {
        const auto iter = texture_map.find(name);
        if (iter == texture_map.end()) {
            const auto texture_path = parent_model_path / name;
            const auto new_texture_path = output_path / texture_path.filename();

            const auto id = load_texture(texture_path, new_texture_path);
            texture_map[name] = id;
            return id;
        }

        return iter->second;
    };

    // Albedo texture
    aiString albedo_path;
    if (mat->GetTexture(AI_MATKEY_BASE_COLOR_TEXTURE, &albedo_path) == aiReturn_SUCCESS) {
        const std::string name = std::string(albedo_path.C_Str());
        auto& id = material->fetch<Phos::UUID>("uAlbedoMap");
        id = get_texture_if_exists_else_add(name);
    }

    // Metallic texture
    aiString metallic_path;
    if (mat->GetTexture(AI_MATKEY_METALLIC_TEXTURE, &metallic_path) == aiReturn_SUCCESS) {
        const std::string name = std::string(metallic_path.C_Str());
        auto& id = material->fetch<Phos::UUID>("uMetallicMap");
        id = get_texture_if_exists_else_add(name);
    }

    // Roughness texture
    aiString roughness_path;
    if (mat->GetTexture(AI_MATKEY_ROUGHNESS_TEXTURE, &roughness_path) == aiReturn_SUCCESS) {
        const std::string name = std::string(roughness_path.C_Str());
        auto& id = material->fetch<Phos::UUID>("uRoughnessMap");
        id = get_texture_if_exists_else_add(name);
    }

    // AO texture
    aiString ao_path;
    if (mat->GetTexture(aiTextureType_LIGHTMAP, 0, &ao_path) == aiReturn_SUCCESS) {
        const std::string name = std::string(ao_path.C_Str());
        auto& id = material->fetch<Phos::UUID>("uAOMap");
        id = get_texture_if_exists_else_add(name);
    }

    // Normal texture
    aiString normal_path;
    if (mat->GetTexture(aiTextureType_NORMALS, 0, &normal_path) == aiReturn_SUCCESS) {
        const std::string name = std::string(normal_path.C_Str());
        auto& id = material->fetch<Phos::UUID>("uNormalMap");
        id = get_texture_if_exists_else_add(name);
    }

    const auto material_path = output_path / ("material_" + std::to_string(idx) + ".psa");
    material->save(material_path);

    return material->get_material_id();
}

Phos::UUID AssimpImporter::load_texture(const std::filesystem::path& texture_path,
                                        const std::filesystem::path& new_texture_path) {
    std::filesystem::copy(texture_path, new_texture_path);

    const auto asset_id = Phos::UUID();

    auto texture_builder = AssetBuilder();

    texture_builder.dump("assetType", "texture");
    texture_builder.dump("id", asset_id);

    texture_builder.dump("path", new_texture_path.filename());

    const auto texture_psa_path = new_texture_path.parent_path() / (new_texture_path.filename().string() + ".psa");
    std::ofstream file(texture_psa_path);
    file << texture_builder;

    return asset_id;
}

AssetBuilder AssimpImporter::process_model_node_r(const aiNode* node,
                                                  const aiScene* scene,
                                                  const std::vector<Phos::UUID>& mesh_ids,
                                                  const std::vector<Phos::UUID>& material_ids) {
    auto node_builder = AssetBuilder();

    if (node->mNumMeshes) {
        const auto mesh = scene->mMeshes[node->mMeshes[0]];
        const auto material_idx = mesh->mMaterialIndex;

        node_builder.dump("mesh", mesh_ids[node->mMeshes[0]]);
        node_builder.dump("material", material_ids[material_idx]);
    }

    if (node->mNumChildren) {
        auto children_builder = AssetBuilder();

        for (std::size_t i = 0; i < node->mNumChildren; ++i) {
            const std::string child_name = "node_" + std::to_string(i);
            const auto child_builder = process_model_node_r(node->mChildren[i], scene, mesh_ids, material_ids);

            children_builder.dump(child_name, child_builder);
        }

        node_builder.dump("children", children_builder);
    }

    return node_builder;
}
