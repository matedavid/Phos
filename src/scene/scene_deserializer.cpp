#include "scene_deserializer.h"

#include <yaml-cpp/yaml.h>

#include "scene/scene.h"
#include "scene/entity_deserializer.h"

#include "asset/asset_manager.h"

#include "renderer/backend/cubemap.h"

namespace Phos {

std::shared_ptr<Scene> SceneDeserializer::deserialize(const std::string& path,
                                                      const std::shared_ptr<AssetManagerBase>& asset_manager) {
    const YAML::Node node = YAML::LoadFile(path);

    const auto name = node["name"].as<std::string>();
    auto scene = std::make_shared<Scene>(name);

    // Load scene config
    const auto config_node = node["config"];
    auto& renderer_config = scene->config();

    renderer_config.bloom_config.enabled = config_node["bloomConfig"]["enabled"].as<bool>();
    renderer_config.bloom_config.threshold = config_node["bloomConfig"]["threshold"].as<float>();

    const auto skybox_id = UUID(config_node["environmentConfig"]["skybox"].as<uint64_t>());
    renderer_config.environment_config.skybox =
        skybox_id == UUID(0) ? nullptr : asset_manager->load_by_id_type<Phos::Cubemap>(skybox_id);

    // Load entities
    const auto entities = node["entities"];
    for (const auto& it : entities) {
        auto entity = EntityDeserializer::deserialize(entities[it.first], scene, asset_manager);
        entity.get_component<UUIDComponent>().uuid = UUID(it.first.as<std::size_t>());
    }

    return scene;
}

} // namespace Phos
