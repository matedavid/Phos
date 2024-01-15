#pragma once

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
};

} // namespace Phos