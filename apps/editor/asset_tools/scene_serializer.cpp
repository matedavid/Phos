#include "scene_serializer.h"

#include <fstream>

#include "asset_tools/entity_serializer.h"
#include "asset_tools/asset_builder.h"

#include "scene/scene.h"
#include "renderer/backend/cubemap.h"

void SceneSerializer::serialize(const std::shared_ptr<Phos::Scene>& scene, const std::filesystem::path& path) {
    auto builder = AssetBuilder();

    builder.dump("assetType", *Phos::AssetType::to_string(Phos::AssetType::Scene));
    builder.dump("name", scene->name());
    builder.dump("id", scene->id);

    // Config
    const auto& config = scene->config();
    auto config_builder = AssetBuilder();

    {
        // Rendering config
        auto rendering_config = AssetBuilder();
        rendering_config.dump("shadowMapResolution", config.rendering_config.shadow_map_resolution);

        config_builder.dump("renderingConfig", rendering_config);
    }

    {
        // Bloom config
        auto bloom_config = AssetBuilder();
        bloom_config.dump("enabled", config.bloom_config.enabled);
        bloom_config.dump("threshold", config.bloom_config.threshold);

        config_builder.dump("bloomConfig", bloom_config);
    }

    {
        // Environment config
        auto environment_config = AssetBuilder();
        const auto skybox_id =
            config.environment_config.skybox != nullptr ? (uint64_t)config.environment_config.skybox->id : 0;
        environment_config.dump("skybox", skybox_id);

        config_builder.dump("environmentConfig", environment_config);
    }

    builder.dump("config", config_builder);

    // Entities
    auto entities_builder = AssetBuilder();

    auto entities = scene->get_all_entities();
    std::ranges::sort(entities, [](const Phos::Entity& a, const Phos::Entity& b) {
        return static_cast<uint64_t>(a.uuid()) < static_cast<uint64_t>(b.uuid());
    });

    for (const auto& entity : entities) {
        auto entity_builder = EntitySerializer::serialize(entity);

        const std::string str_id = std::to_string(static_cast<uint64_t>(entity.uuid()));
        entities_builder.dump(str_id, entity_builder);
    }

    builder.dump("entities", entities_builder);

    std::ofstream file(path);
    file << builder;
}
