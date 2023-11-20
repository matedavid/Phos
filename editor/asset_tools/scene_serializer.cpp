#include "scene_serializer.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

#include "asset_tools/entity_serializer.h"
#include "asset_tools/asset_dumping_utils.h"

#include "scene/scene.h"
#include "renderer/backend/cubemap.h"

void SceneSerializer::serialize(const std::shared_ptr<Phos::Scene>& scene, const std::filesystem::path& path) {
    const auto& entities = scene->get_all_entities();

    YAML::Emitter out;
    out << YAML::BeginMap;
    AssetDumpingUtils::emit_yaml(out, "assetType", "scene");
    AssetDumpingUtils::emit_yaml(out, "name", scene->name());
    AssetDumpingUtils::emit_yaml(out, "id", (uint64_t)scene->id);

    // Emit config
    const auto& config = scene->config();

    AssetDumpingUtils::emit_yaml(out, "config");
    out << YAML::BeginMap;

    AssetDumpingUtils::emit_yaml(out, "bloomConfig");
    {
        out << YAML::BeginMap;

        AssetDumpingUtils::emit_yaml(out, "enabled", config.bloom_config.enabled);
        AssetDumpingUtils::emit_yaml(out, "threshold", config.bloom_config.threshold);

        out << YAML::EndMap;
    }

    AssetDumpingUtils::emit_yaml(out, "environmentConfig");
    {
        out << YAML::BeginMap;

        const auto skybox_id =
            config.environment_config.skybox != nullptr ? (uint64_t)config.environment_config.skybox->id : 0;
        AssetDumpingUtils::emit_yaml(out, "skybox", skybox_id);

        out << YAML::EndMap;
    }

    out << YAML::EndMap;

    // Emit entities
    AssetDumpingUtils::emit_yaml(out, "entities");
    out << YAML::BeginMap;

    for (const auto& entity : entities)
        EntitySerializer::serialize(out, entity);

    out << YAML::EndMap << YAML::EndMap;

    std::ofstream file(path);
    file << out.c_str();
}
