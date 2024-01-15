#include "prefab_loader.h"

#include <yaml-cpp/yaml.h>

#include "asset/asset_manager.h"
#include "asset/prefab_asset.h"

#include "scene/entity_deserializer.h"

namespace Phos {

Entity PrefabLoader::load(const PrefabAsset& prefab,
                          const std::shared_ptr<Scene>& scene,
                          const std::shared_ptr<AssetManagerBase>& asset_manager) {
    const auto components_node = YAML::Load(prefab.components_string_representation);
    return EntityDeserializer::deserialize(components_node, UUID(), scene, asset_manager.get());
}

} // namespace Phos
