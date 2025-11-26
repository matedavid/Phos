#include "editor_material_helper.h"

#include <fstream>

#include "utility/logging.h"

#include "asset_tools/asset_builder.h"

#include "core/uuid.h"
#include "managers/shader_manager.h"
#include "asset/asset_parsing_utils.h"
#include "asset/asset.h"

#include "renderer/backend/renderer.h"

std::shared_ptr<EditorMaterialHelper> EditorMaterialHelper::create(const std::shared_ptr<Phos::Shader>& shader,
                                                                   std::string name) {
    return std::shared_ptr<EditorMaterialHelper>(new EditorMaterialHelper(shader, std::move(name)));
}

// create constructor
EditorMaterialHelper::EditorMaterialHelper(const std::shared_ptr<Phos::Shader>& shader, std::string name)
      : m_material_name(std::move(name)), m_material_id(Phos::UUID()) {
    m_properties = shader->get_shader_properties();
    for (const auto& property : m_properties) {
        input_default_value(property.name, property.type);
    }
}

std::shared_ptr<EditorMaterialHelper> EditorMaterialHelper::open(const std::filesystem::path& path) {
    return std::shared_ptr<EditorMaterialHelper>(new EditorMaterialHelper(path));
}

// open constructor
EditorMaterialHelper::EditorMaterialHelper(const std::filesystem::path& path) : m_path(path) {
    const auto node = YAML::LoadFile(m_path.string());

    m_material_name = path.stem().string();
    m_material_id = Phos::UUID(node["id"].as<uint64_t>());

    const auto shader_type = node["shader"]["type"].as<std::string>();
    PHOS_ASSERT(shader_type == "builtin", "Only builtin shaders supported at the moment");

    const auto shader_name = node["shader"]["name"].as<std::string>();
    const auto shader = Phos::Renderer::shader_manager()->get_builtin_shader(shader_name);

    const auto properties_node = node["properties"];

    m_properties = shader->get_shader_properties();
    for (const auto& property : m_properties) {
        if (!properties_node[property.name]) {
            input_default_value(property.name, property.type);
            continue;
        }

        m_name_to_type[property.name] = property.type;

        const auto data_node = properties_node[property.name]["data"];

        switch (property.type) {
        case Phos::ShaderProperty::Type::Float:
            m_float_properties[property.name] = Phos::AssetParsingUtils::parse_numeric<float>(data_node);
            break;
        case Phos::ShaderProperty::Type::Vec3:
            m_vec3_properties[property.name] = Phos::AssetParsingUtils::parse_vec3(data_node);
            break;
        case Phos::ShaderProperty::Type::Vec4:
            m_vec4_properties[property.name] = Phos::AssetParsingUtils::parse_vec4(data_node);
            break;
        case Phos::ShaderProperty::Type::Texture:
            m_texture_properties[property.name] = Phos::AssetParsingUtils::parse_uuid(data_node);
            break;
        }
    }
}

void EditorMaterialHelper::save() const {
    if (!std::filesystem::exists(m_path)) {
        PHOS_LOG_ERROR("Could not save material {} because path is not set", m_material_name);
        return;
    }

    save(m_path);
}

void EditorMaterialHelper::save(const std::filesystem::path& path) const {
    auto material_builder = AssetBuilder();

    material_builder.dump("assetType", *Phos::AssetType::to_string(Phos::AssetType::Material));
    material_builder.dump("id", m_material_id);
    material_builder.dump("name", m_material_name);

    {
        auto shader_builder = AssetBuilder();
        shader_builder.dump("type", "builtin");
        shader_builder.dump("name", "PBR.Geometry.Deferred");

        material_builder.dump("shader", shader_builder);
    }

    {
        auto properties_builder = AssetBuilder();

        for (const auto& property : m_properties) {
            auto property_builder = AssetBuilder();

            if (property.type == Phos::ShaderProperty::Type::Float) {
                property_builder.dump("type", "float");
                property_builder.dump("data", m_float_properties.at(property.name));
            } else if (property.type == Phos::ShaderProperty::Type::Vec3) {
                property_builder.dump("type", "vec3");
                property_builder.dump("data", m_vec3_properties.at(property.name));
            } else if (property.type == Phos::ShaderProperty::Type::Vec4) {
                property_builder.dump("type", "vec4");
                property_builder.dump("data", m_vec4_properties.at(property.name));
            } else if (property.type == Phos::ShaderProperty::Type::Texture) {
                property_builder.dump("type", "texture");
                property_builder.dump("data", m_texture_properties.at(property.name));
            }

            properties_builder.dump(property.name, property_builder);
        }

        material_builder.dump("properties", properties_builder);
    }

    std::ofstream output_file(path);
    output_file << material_builder;
}

void EditorMaterialHelper::input_default_value(const std::string& property_name, Phos::ShaderProperty::Type type) {
    constexpr float DEFAULT_FLOAT_VALUE = 1.0f;

    m_name_to_type[property_name] = type;

    switch (type) {
    case Phos::ShaderProperty::Type::Float:
        m_float_properties[property_name] = DEFAULT_FLOAT_VALUE;
        if (property_name == "uMaterialInfo.emissionIntensity") // @TODO: Ugly hack, but necessary
            m_float_properties[property_name] = 0.0f;
        break;
    case Phos::ShaderProperty::Type::Vec3:
        m_vec3_properties[property_name] = glm::vec3(DEFAULT_FLOAT_VALUE);
        break;
    case Phos::ShaderProperty::Type::Vec4:
        m_vec4_properties[property_name] = glm::vec4(DEFAULT_FLOAT_VALUE);
        break;
    case Phos::ShaderProperty::Type::Texture:
        m_texture_properties[property_name] = Phos::UUID(0);
        break;
    }
}
