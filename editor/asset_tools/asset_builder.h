#pragma once

#include <stack>
#include <filesystem>
#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>

#include "utility/logging.h"
#include "core/uuid.h"

class AssetBuilder {
  public:
    AssetBuilder() { m_nodes.emplace(); }

    template <typename T>
    void dump(const std::string& key, const T& value) {
        auto& node = m_nodes.top();
        node[key] = value;
    }

    template <typename T>
    void dump(const std::string& key, const std::vector<T>& value) {
        auto list_node = YAML::Node();
        for (const auto& val : value)
            list_node.push_back(val);

        auto& node = m_nodes.top();
        node[key] = list_node;
    }

    void dump(const std::string& key, const std::filesystem::path& value) {
        auto& node = m_nodes.top();
        node[key] = value.string();
    }

    void dump(const std::string& key, const glm::vec3& value) {
        auto data_node = YAML::Node();
        data_node["x"] = value.x;
        data_node["y"] = value.y;
        data_node["z"] = value.z;

        auto& node = m_nodes.top();
        node[key] = data_node;
    }

    void dump(const std::string& key, const glm::vec4& value) {
        auto data_node = YAML::Node();
        data_node["x"] = value.x;
        data_node["y"] = value.y;
        data_node["z"] = value.z;
        data_node["w"] = value.w;

        auto& node = m_nodes.top();
        node[key] = data_node;
    }

    void dump(const std::string& key, const Phos::UUID& value) {
        auto& node = m_nodes.top();
        node[key] = static_cast<uint64_t>(value);
    }

    void dump(const std::string& key, const AssetBuilder& value) {
        auto& node = m_nodes.top();
        node[key] = value.get_top();
    }

    friend std::ostream& operator<<(std::ostream& os, const AssetBuilder& builder) {
        os << YAML::Dump(builder.get_top());
        return os;
    }

  private:
    std::stack<YAML::Node> m_nodes{};

    [[nodiscard]] const YAML::Node& get_top() const {
        if (m_nodes.size() != 1) {
            PHOS_LOG_WARNING("Top node is not root node");
        }

        return m_nodes.top();
    }
};
