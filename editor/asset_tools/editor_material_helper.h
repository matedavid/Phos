#pragma once

#include "core.h"

#include <filesystem>
#include <glm/glm.hpp>

#include "core/uuid.h"

namespace Phos {

// Forward declarations
class Shader;
struct ShaderProperty;

} // namespace Phos

namespace YAML {

// Forward declarations
class Node;

} // namespace YAML

class EditorMaterialHelper {
  public:
    ~EditorMaterialHelper() = default;

    [[nodiscard]] static std::shared_ptr<EditorMaterialHelper> create(const std::shared_ptr<Phos::Shader>& shader);
    [[nodiscard]] static std::shared_ptr<EditorMaterialHelper> open(const std::filesystem::path& path);

    template <typename T>
    T& fetch(const std::string& property_name);

    template <typename T>
    void modify(const std::string& property_name, T& value);

    void save(const std::filesystem::path& path) const;

    [[nodiscard]] const std::vector<Phos::ShaderProperty>& get_properties() const { return m_properties; }

  private:
    std::vector<Phos::ShaderProperty> m_properties;

    std::unordered_map<std::string, float> m_float_properties;
    std::unordered_map<std::string, glm::vec3> m_vec3_properties;
    std::unordered_map<std::string, glm::vec4> m_vec4_properties;
    std::unordered_map<std::string, Phos::UUID> m_texture_properties;

    explicit EditorMaterialHelper(const std::shared_ptr<Phos::Shader>& shader);
    explicit EditorMaterialHelper(const std::filesystem::path& path);

    [[nodiscard]] static float parse_float(const YAML::Node& node);
    [[nodiscard]] static glm::vec3 parse_vec3(const YAML::Node& node);
    [[nodiscard]] static glm::vec4 parse_vec4(const YAML::Node& node);
    [[nodiscard]] static Phos::UUID parse_texture(const YAML::Node& node);
};
