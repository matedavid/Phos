#pragma once

#include "core.h"

#include <filesystem>
#include <glm/glm.hpp>

#include "core/uuid.h"

#include "renderer/backend/shader.h"

namespace YAML {

// Forward declarations
class Node;

} // namespace YAML

class EditorMaterialHelper {
  public:
    ~EditorMaterialHelper() = default;

    [[nodiscard]] static std::shared_ptr<EditorMaterialHelper> create(const std::shared_ptr<Phos::Shader>& shader,
                                                                      std::string name);
    [[nodiscard]] static std::shared_ptr<EditorMaterialHelper> open(const std::filesystem::path& path);

    template <typename T>
    [[nodiscard]] T& fetch(const std::string& property_name) {
        const auto type = m_name_to_type[property_name];

        switch (type) {
        default:
        case Phos::ShaderProperty::Type::Float:
            return (T&)m_float_properties[property_name];
        case Phos::ShaderProperty::Type::Vec3:
            return (T&)m_vec3_properties[property_name];
        case Phos::ShaderProperty::Type::Vec4:
            return (T&)m_vec4_properties[property_name];
        case Phos::ShaderProperty::Type::Texture:
            return (T&)m_texture_properties[property_name];
        }
    }

    void save() const;
    void save(const std::filesystem::path& path) const;

    [[nodiscard]] std::string get_material_name() const { return m_material_name; }
    [[nodiscard]] const std::vector<Phos::ShaderProperty>& get_properties() const { return m_properties; }

  private:
    std::string m_material_name;
    std::filesystem::path m_path;
    Phos::UUID m_material_id;

    std::vector<Phos::ShaderProperty> m_properties;
    std::unordered_map<std::string, Phos::ShaderProperty::Type> m_name_to_type;

    std::unordered_map<std::string, float> m_float_properties;
    std::unordered_map<std::string, glm::vec3> m_vec3_properties;
    std::unordered_map<std::string, glm::vec4> m_vec4_properties;
    std::unordered_map<std::string, Phos::UUID> m_texture_properties;

    explicit EditorMaterialHelper(const std::shared_ptr<Phos::Shader>& shader, std::string name);
    explicit EditorMaterialHelper(const std::filesystem::path& path);

    void input_default_value(const std::string& property_name, Phos::ShaderProperty::Type type);

    [[nodiscard]] static float parse_float(const YAML::Node& node);
    [[nodiscard]] static glm::vec3 parse_vec3(const YAML::Node& node);
    [[nodiscard]] static glm::vec4 parse_vec4(const YAML::Node& node);
    [[nodiscard]] static Phos::UUID parse_texture(const YAML::Node& node);
};
