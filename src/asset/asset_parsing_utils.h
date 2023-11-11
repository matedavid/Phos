#pragma once

#include "core.h"
#include "core/uuid.h"

#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>

namespace Phos {

class AssetParsingUtils {
  public:
    template <typename T>
    static T parse_numeric(const YAML::Node& node) {
        return node.as<T>();
    }

    static glm::vec3 parse_vec3(const YAML::Node& node) {
        return {
            node["x"].as<float>(),
            node["y"].as<float>(),
            node["z"].as<float>(),
        };
    }

    static glm::vec4 parse_vec4(const YAML::Node& node) {
        return {
            node["x"].as<float>(),
            node["y"].as<float>(),
            node["z"].as<float>(),
            node["w"].as<float>(),
        };
    }

    static std::string parse_string(const YAML::Node& node) { return node.as<std::string>(); }

    static UUID parse_uuid(const YAML::Node& node) { return UUID(node.as<uint64_t>()); }

    template <typename T>
    static void dump(YAML::Emitter& out, const std::string& key, const T& value) {
        out << YAML::Key << key << YAML::Value << value;
    }

    static void dump_vec3(YAML::Emitter& out, const std::string& key, const glm::vec3& value) {
        out << YAML::Key << key << YAML::BeginMap;

        out << YAML::Key << "x" << YAML::Value << value.x;
        out << YAML::Key << "y" << YAML::Value << value.y;
        out << YAML::Key << "z" << YAML::Value << value.z;

        out << YAML::EndMap;
    }

    static void dump_vec4(YAML::Emitter& out, const std::string& key, const glm::vec4& value) {
        out << YAML::Key << key << YAML::BeginMap;

        out << YAML::Key << "x" << YAML::Value << value.x;
        out << YAML::Key << "y" << YAML::Value << value.y;
        out << YAML::Key << "z" << YAML::Value << value.z;
        out << YAML::Key << "w" << YAML::Value << value.w;

        out << YAML::EndMap;
    }
};

} // namespace Phos