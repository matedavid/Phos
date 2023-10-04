#include "editor_material_helper.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

#include "core/uuid.h"
#include "managers/shader_manager.h"

#include "renderer/backend/shader.h"
#include "renderer/backend/renderer.h"

std::shared_ptr<EditorMaterialHelper> EditorMaterialHelper::create(const std::shared_ptr<Phos::Shader>& shader) {
    return std::shared_ptr<EditorMaterialHelper>(new EditorMaterialHelper(shader));
}

// create constructor
EditorMaterialHelper::EditorMaterialHelper(const std::shared_ptr<Phos::Shader>& shader) {
    constexpr float DEFAULT_FLOAT_VALUE = 1.0f;

    m_properties = shader->get_shader_properties();
    for (const auto& property : m_properties) {
        switch (property.type) {
        case Phos::ShaderProperty::Type::Float:
            m_float_properties[property.name] = DEFAULT_FLOAT_VALUE;
            break;
        case Phos::ShaderProperty::Type::Vec3:
            m_vec3_properties[property.name] = glm::vec3(DEFAULT_FLOAT_VALUE);
            break;
        case Phos::ShaderProperty::Type::Vec4:
            m_vec4_properties[property.name] = glm::vec4(DEFAULT_FLOAT_VALUE);
            break;
        case Phos::ShaderProperty::Type::Texture:
            m_texture_properties[property.name] = Phos::UUID(0);
            break;
        }
    }
}

std::shared_ptr<EditorMaterialHelper> EditorMaterialHelper::open(const std::filesystem::path& path) {
    return std::shared_ptr<EditorMaterialHelper>(new EditorMaterialHelper(path));
}

// open constructor
EditorMaterialHelper::EditorMaterialHelper(const std::filesystem::path& path) {
    const auto node = YAML::LoadFile(path);

    const auto shader_type = node["shader"]["type"].as<std::string>();
    PS_ASSERT(shader_type == "builtin", "Only builtin shaders supported at the moment")

    const auto shader_name = node["shader"]["name"].as<std::string>();
    const auto shader = Phos::Renderer::shader_manager()->get_builtin_shader(shader_name);

    const auto properties_node = node["properties"];

    m_properties = shader->get_shader_properties();
    for (const auto& property : m_properties) {
        const auto data_node = properties_node[property.name]["data"];

        switch (property.type) {
        case Phos::ShaderProperty::Type::Float:
            m_float_properties[property.name] = parse_float(data_node);
            break;
        case Phos::ShaderProperty::Type::Vec3:
            m_vec3_properties[property.name] = parse_vec3(data_node);
            break;
        case Phos::ShaderProperty::Type::Vec4:
            m_vec4_properties[property.name] = parse_vec4(data_node);
            break;
        case Phos::ShaderProperty::Type::Texture:
            m_texture_properties[property.name] = parse_texture(data_node);
            break;
        }
    }
}

void EditorMaterialHelper::save(const std::filesystem::path& path) const {
    YAML::Emitter out;
    out << YAML::BeginMap;

    out << YAML::Key << "assetType" << YAML::Value << "material";
    out << YAML::Key << "id" << YAML::Value << (uint64_t)Phos::UUID();

    out << YAML::Key << "name" << YAML::Value << path.stem().string();

    out << YAML::Key << "shader";
    out << YAML::BeginMap;
    {
        out << YAML::Key << "type" << YAML::Value << "builtin";
        out << YAML::Key << "name" << YAML::Value << "PBR.Geometry.Deferred";
    }
    out << YAML::EndMap;

    out << YAML::Key << "properties";
    out << YAML::BeginMap;

    for (const auto& property : m_properties) {
        out << YAML::Key << property.name << YAML::BeginMap;

        if (property.type == Phos::ShaderProperty::Type::Float) {
            out << YAML::Key << "type" << YAML::Value << "float";
            out << YAML::Key << "data" << YAML::Value << m_float_properties.at(property.name);
        } else if (property.type == Phos::ShaderProperty::Type::Vec3) {
            out << YAML::Key << "type" << YAML::Value << "vec3";

            out << YAML::Key << "data" << YAML::BeginMap;
            {
                const auto data = m_vec3_properties.at(property.name);

                out << YAML::Key << "x" << YAML::Value << data.x;
                out << YAML::Key << "y" << YAML::Value << data.y;
                out << YAML::Key << "z" << YAML::Value << data.z;
            }
            out << YAML::EndMap;
        } else if (property.type == Phos::ShaderProperty::Type::Vec4) {
            out << YAML::Key << "type" << YAML::Value << "vec3";

            out << YAML::Key << "data" << YAML::BeginMap;
            {
                const auto data = m_vec4_properties.at(property.name);

                out << YAML::Key << "x" << YAML::Value << data.x;
                out << YAML::Key << "y" << YAML::Value << data.y;
                out << YAML::Key << "z" << YAML::Value << data.z;
                out << YAML::Key << "w" << YAML::Value << data.w;
            }
            out << YAML::EndMap;
        } else if (property.type == Phos::ShaderProperty::Type::Texture) {
            out << YAML::Key << "type" << YAML::Value << "texture";
            out << YAML::Key << "data" << YAML::Value << (uint64_t)m_texture_properties.at(property.name);
        }

        out << YAML::EndMap;
    }

    out << YAML::EndMap << YAML::EndMap;

    std::ofstream output_file(path);
    output_file << out.c_str();
}

float EditorMaterialHelper::parse_float(const YAML::Node& node) {
    return node.as<float>();
}

glm::vec3 EditorMaterialHelper::parse_vec3(const YAML::Node& node) {
    return {node["x"].as<float>(), node["y"].as<float>(), node["z"].as<float>()};
}

glm::vec4 EditorMaterialHelper::parse_vec4(const YAML::Node& node) {
    return {node["x"].as<float>(), node["y"].as<float>(), node["z"].as<float>(), node["w"].as<float>()};
}

Phos::UUID EditorMaterialHelper::parse_texture(const YAML::Node& node) {
    return Phos::UUID(node.as<uint64_t>());
}
