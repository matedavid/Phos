#pragma once

#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>

class AssetDumpingUtils {
  public:
    template <typename F>
    static void emit_yaml(YAML::Emitter& out, const F& key) {
        out << YAML::Key << key;
        out << YAML::Value;
    }

    template <typename F, typename S>
    static void emit_yaml(YAML::Emitter& out, const F& key, const S& value) {
        out << YAML::Key << key;
        out << YAML::Value << value;
    }

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