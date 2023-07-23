#include <iostream>
#include <filesystem>
#include <random>
#include <fstream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <yaml-cpp/yaml.h>

// constexpr uint32_t import_flags =
//     aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph;

constexpr uint32_t import_flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes;

std::filesystem::path s_output_folder;
std::filesystem::path s_containing_folder;
std::filesystem::path s_new_model_path;

uint64_t uuid() {
    static std::random_device s_random_device;
    static std::mt19937_64 s_engine(s_random_device());
    static std::uniform_int_distribution<uint64_t> s_uniform_distribution;

    return s_uniform_distribution(s_engine);
}

template <typename F>
void emit_yaml(YAML::Emitter& out, const F& key) {
    out << YAML::Key << key;
    out << YAML::Value;
}

template <typename F, typename S>
void emit_yaml(YAML::Emitter& out, const F& key, const S& value) {
    out << YAML::Key << key;
    out << YAML::Value << value;
}

std::unordered_map<std::string, uint64_t> s_texture_to_id;

uint64_t get_texture(const std::string& path) {
    if (s_texture_to_id.contains(path)) {
        return s_texture_to_id[path];
    }

    const auto complete_path = s_containing_folder / path;
    const auto output_path = s_output_folder / std::filesystem::path(path).filename();

    if (!exists(output_path))
        std::filesystem::copy(complete_path, output_path);

    const auto id = uuid();

    YAML::Emitter out;
    out << YAML::BeginMap;

    emit_yaml(out, "assetType", "texture");
    emit_yaml(out, "id", id);
    emit_yaml(out, "path", output_path.filename());

    out << YAML::EndMap;

    std::ofstream file(std::string(output_path) + ".psa");
    file << out.c_str();

    s_texture_to_id[path] = id;

    return id;
}

struct AssetInformation {
    uint64_t id;
    std::string output;
};

AssetInformation load_mesh(const std::size_t idx) {
    const uint64_t id = uuid();

    YAML::Emitter out;

    out << YAML::BeginMap;

    emit_yaml(out, "assetType", "mesh");
    emit_yaml(out, "id", id);
    emit_yaml(out, "path", s_new_model_path);
    emit_yaml(out, "index", idx);

    out << YAML::EndMap;

    return {.id = id, .output = out.c_str()};
}

AssetInformation load_material(const aiMaterial* mat) {
    const uint64_t id = uuid();

    YAML::Emitter out;

    out << YAML::BeginMap;

    emit_yaml(out, "assetType", "material");
    emit_yaml(out, "id", id);
    emit_yaml(out, "name", mat->GetName().C_Str());

    emit_yaml(out, "shader");
    out << YAML::BeginMap;
    emit_yaml(out, "type", "builtin");
    emit_yaml(out, "name", "PBR.Geometry.Deferred");
    out << YAML::EndMap; // end shader

    emit_yaml(out, "properties");
    out << YAML::BeginMap;
    {
        const auto emit_mat_property = [&]<typename T>(const std::string& name, const std::string& type, const T& dt) {
            emit_yaml(out, name);
            out << YAML::BeginMap;
            emit_yaml(out, "type", type);
            emit_yaml(out, "data", dt);
            out << YAML::EndMap;
        };

        // Albedo texture
        aiString albedo_path;
        if (mat->GetTexture(AI_MATKEY_BASE_COLOR_TEXTURE, &albedo_path) == aiReturn_SUCCESS) {
            const std::string path = std::string(albedo_path.C_Str());
            emit_mat_property("uAlbedoMap", "texture", get_texture(path));
        }

        // Metallic texture
        aiString metallic_path;
        if (mat->GetTexture(AI_MATKEY_METALLIC_TEXTURE, &metallic_path) == aiReturn_SUCCESS) {
            const std::string path = std::string(metallic_path.C_Str());
            emit_mat_property("uMetallicMap", "texture", get_texture(path));
        }

        // Roughness texture
        aiString roughness_path;
        if (mat->GetTexture(AI_MATKEY_ROUGHNESS_TEXTURE, &roughness_path) == aiReturn_SUCCESS) {
            const std::string path = std::string(roughness_path.C_Str());
            emit_mat_property("uRoughnessMap", "texture", get_texture(path));
        }

        // AO texture
        aiString ao_path;
        if (mat->GetTexture(aiTextureType_LIGHTMAP, 0, &ao_path) == aiReturn_SUCCESS) {
            const std::string path = std::string(ao_path.C_Str());
            emit_mat_property("uAOMap", "texture", get_texture(path));
        }

        // Normal texture
        aiString normal_path;
        if (mat->GetTexture(aiTextureType_NORMALS, 0, &normal_path) == aiReturn_SUCCESS) {
            const std::string path = std::string(normal_path.C_Str());
            emit_mat_property("uNormalMap", "texture", get_texture(path));
        }
    }

    out << YAML::EndMap; // end properties

    out << YAML::EndMap;

    return {.id = id, .output = out.c_str()};
}

void process_model_node_r(YAML::Emitter& out,
                          const aiNode* node,
                          const aiScene* scene,
                          const std::vector<AssetInformation>& info_meshes,
                          const std::vector<AssetInformation>& info_materials,
                          int child_number) {
    const std::string node_name = "node_" + std::to_string(child_number);
    emit_yaml(out, node_name);
    out << YAML::BeginMap;

    if (node->mNumMeshes) {
        const auto mesh = scene->mMeshes[node->mMeshes[0]];
        const auto material_idx = mesh->mMaterialIndex;

        emit_yaml(out, "mesh", info_meshes[node->mMeshes[0]].id);
        emit_yaml(out, "material", info_materials[material_idx].id);
    }

    if (node->mNumChildren) {
        emit_yaml(out, "children");
        out << YAML::BeginMap;

        for (std::size_t i = 0; i < node->mNumChildren; ++i) {
            process_model_node_r(out, node->mChildren[i], scene, info_meshes, info_materials, i);
        }

        out << YAML::EndMap;
    }

    out << YAML::EndMap;
}

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage:\t./AssimpImporter <model_path>\n";
        return 1;
    }

    const std::filesystem::path path = argv[1];
    s_containing_folder = std::filesystem::path(path).parent_path();

    if (!exists(path)) {
        std::cout << "File: " << path << " does not exist\n";
        return 1;
    }

    s_output_folder = path.string() + "_imported";
    if (argc == 3)
        s_output_folder = argv[2];

    if (!exists(s_output_folder)) {
        create_directory(s_output_folder);
    } else if (!is_directory(s_output_folder)) {
        std::cout << "Output folder: " << s_output_folder << " is not a directory\n";
        return 1;
    }

    s_new_model_path = path.filename();

    if (!exists(s_new_model_path))
        std::filesystem::copy(path, s_output_folder / s_new_model_path);

    if (path.extension() == "gltf") {
        const auto original_bin_file = path.parent_path() / (path.stem().string() + ".bin");
        const auto dest_bin_file = s_output_folder / (path.stem().string() + ".bin");
        std::filesystem::copy(original_bin_file, dest_bin_file);
    }

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path.c_str(), import_flags);

    if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        std::cout << "Failed to load model...\n";
        return 1;
    }

    std::cout << "Model information:\n";
    std::cout << "    Num meshes: " << scene->mNumMeshes << "\n";
    std::cout << "    Num materials: " << scene->mNumMaterials << "\n";

    // Load meshes
    std::vector<AssetInformation> info_meshes;
    for (std::size_t i = 0; i < scene->mNumMeshes; ++i) {
        const auto info = load_mesh(i);
        info_meshes.push_back(info);

        // const auto mesh_name = std::string(scene->mMeshes[i]->mName.C_Str());
        const auto mesh_name = "mesh_" + std::to_string(i);

        const std::filesystem::path mesh_path = s_output_folder / (mesh_name + ".psa");
        std::ofstream file(mesh_path);
        file << info.output;
    }

    // Load materials
    std::vector<AssetInformation> info_materials;
    for (std::size_t i = 0; i < scene->mNumMaterials; ++i) {
        const auto info = load_material(scene->mMaterials[i]);
        info_materials.push_back(info);

        // const auto material_name = std::string(scene->mMaterials[i]->GetName().C_Str());
        const auto material_name = "material_" + std::to_string(i);

        const std::filesystem::path material_path = s_output_folder / (material_name + ".psa");
        std::ofstream file(material_path);
        file << info.output;
    }

    // Create model information
    YAML::Emitter out;
    out << YAML::BeginMap;

    emit_yaml(out, "assetType", "model");
    emit_yaml(out, "id", uuid());

    process_model_node_r(out, scene->mRootNode, scene, info_meshes, info_materials, 0);

    out << YAML::EndMap;

    const auto model_psa_path = s_output_folder / (path.filename().string() + ".psa");
    std::ofstream file(model_psa_path);
    file << out.c_str();

    return 0;
}