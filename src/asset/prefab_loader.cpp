#include "prefab_loader.h"

#include <yaml-cpp/yaml.h>

#include "asset/asset_manager.h"
#include "asset/prefab_asset.h"

#include "scene/scene.h"
#include "scene/entity_deserializer.h"

namespace Phos {

Entity Phos::PrefabLoader::load(const UUID& prefab_id,
                                const std::shared_ptr<Scene>& scene,
                                const std::shared_ptr<AssetManagerBase>& asset_manager) {
    const auto prefab = asset_manager->load_by_id_type<PrefabAsset>(prefab_id);

    const auto components_node = YAML::Load(prefab->components_string_representation);
    return EntityDeserializer::deserialize(components_node, scene, asset_manager);
}

} // namespace Phos
