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
#include "asset/asset.h"

void AssimpImporter::import_model(const AssetImporter::ImportModelInfo& import_info,
                                  const std::filesystem::path& containing_folder) {
    PHOS_ASSERT(
        std::filesystem::exists(import_info.path), "Model path: '{}' does not exist", import_info.path.string());

    uint32_t import_flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes;
    if (import_info.import_static) {
        import_flags |= aiProcess_OptimizeGraph;
    }

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(import_info.path.c_str(), import_flags);

    if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        PHOS_LOG_ERROR("Failed to load model: '{}'", import_info.path.string());
        return;
    }

    // Copy original asset to output_folder_path
    const auto output_model_path = containing_folder / import_info.path.filename();
    if (!std::filesystem::exists(output_model_path)) {
        std::filesystem::copy(import_info.path, output_model_path);
    } else {
        PHOS_LOG_WARNING("Model file '{}' already exists in project path '{}'",
                         import_info.path.filename().string(),
                         containing_folder.string());
    }

    // Copy additional files based on model extension
    copy_additional_files(import_info.path, containing_folder);

    // Load meshes
    std::vector<Phos::UUID> mesh_ids;
    if (import_info.import_static) {
        AssetBuilder static_mesh_builder{};

        const Phos::UUID id{};

        static_mesh_builder.dump("assetType", *Phos::AssetType::to_string(Phos::AssetType::StaticMesh));
        static_mesh_builder.dump("id", id);

        static_mesh_builder.dump("source", std::filesystem::relative(output_model_path, containing_folder));

        const auto static_mesh_path = containing_folder / (import_info.path.stem().string() + ".psa");
        std::ofstream file(static_mesh_path);
        file << static_mesh_builder;

        mesh_ids.push_back(id);
    } else {
        PHOS_FAIL("Not implemented");
    }

    // Load textures
    std::unordered_map<std::string, Phos::UUID> texture_map;
    for (std::size_t i = 0; i < scene->mNumTextures; ++i) {
        const auto texture = scene->mTextures[i];
        const auto texture_path = import_info.path.parent_path() / texture->mFilename.C_Str();
        const auto new_texture_path = containing_folder / std::filesystem::path(texture->mFilename.C_Str()).filename();

        if (std::filesystem::exists(new_texture_path))
            continue;

        texture_map[texture->mFilename.C_Str()] = load_texture(texture_path, new_texture_path);
    }

    std::vector<Phos::UUID> material_ids;
    if (import_info.import_materials) {
        // Load materials
        for (std::size_t i = 0; i < scene->mNumMaterials; ++i) {
            const auto id =
                load_material(scene->mMaterials[i], texture_map, import_info.path.parent_path(), containing_folder);

            if (id != Phos::UUID(0))
                material_ids.push_back(id);
        }
    }
}

void AssimpImporter::copy_additional_files(const std::filesystem::path& model_path,
                                           const std::filesystem::path& containing_folder) {
    const auto ext = model_path.extension();

    if (ext == ".obj") {
        const auto mtl_path = model_path.parent_path() / (model_path.stem().string() + ".mtl");
        if (!std::filesystem::exists(mtl_path)) {
            PHOS_LOG_WARNING("No .mtl file found for .obj model in folder: {}", model_path.parent_path().string());
            return;
        }

        const auto output_mtl_path = containing_folder / (model_path.stem().string() + ".mtl");
        std::filesystem::copy(mtl_path, output_mtl_path);
    } else if (ext == ".gltf") {
        const auto bin_path = model_path.parent_path() / (model_path.stem().string() + ".bin");
        if (!std::filesystem::exists(bin_path)) {
            PHOS_LOG_WARNING("No .bin file found for .gltf model in folder: {}", model_path.parent_path().string());
            return;
        }

        const auto output_bin_path = containing_folder / (model_path.stem().string() + ".bin");
        std::filesystem::copy(bin_path, output_bin_path);
    }
}

Phos::UUID AssimpImporter::load_material(const aiMaterial* mat,
                                         std::unordered_map<std::string, Phos::UUID>& texture_map,
                                         const std::filesystem::path& parent_model_path,
                                         const std::filesystem::path& output_path) {
    const auto shader = Phos::Renderer::shader_manager()->get_builtin_shader("PBR.Geometry.Deferred");

    const std::string mat_name = mat->GetName().C_Str();
    if (mat_name.empty())
        return Phos::UUID(0);

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

    const auto material_path = output_path / (mat_name + ".psa");
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
