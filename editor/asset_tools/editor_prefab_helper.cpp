#include "editor_prefab_helper.h"

#include <fstream>

#include "asset_tools/asset_builder.h"
#include "asset_tools/entity_serializer.h"

#include "asset/editor_asset_manager.h"
#include "asset/prefab_asset.h"
#include "asset/prefab_loader.h"

#include "scene/scene.h"

std::shared_ptr<EditorPrefabHelper> EditorPrefabHelper::create(
    std::shared_ptr<Phos::EditorAssetManager> asset_manager) {
    return std::shared_ptr<EditorPrefabHelper>(new EditorPrefabHelper(std::move(asset_manager)));
}

// create constructor
EditorPrefabHelper::EditorPrefabHelper(std::shared_ptr<Phos::EditorAssetManager> asset_manager)
      : m_asset_manager(std::move(asset_manager)) {
    // @TODO:
}

std::shared_ptr<EditorPrefabHelper> EditorPrefabHelper::open(const std::filesystem::path& path,
                                                             std::shared_ptr<Phos::EditorAssetManager> asset_manager) {
    return std::shared_ptr<EditorPrefabHelper>(new EditorPrefabHelper(path, std::move(asset_manager)));
}

// open constructor
EditorPrefabHelper::EditorPrefabHelper(const std::filesystem::path& path,
                                       std::shared_ptr<Phos::EditorAssetManager> asset_manager)
      : m_asset_manager(std::move(asset_manager)), m_path(path) {
    m_prefab_scene = std::make_shared<Phos::Scene>("Prefab scene");

    m_prefab_id = m_asset_manager->get_asset_id(path);
    if (m_prefab_id == Phos::UUID(0)) {
        PS_ERROR("[EditorPrefabHelper::ctor] Cannot open prefab, no prefab in path: {}", m_path.string());
        return;
    }

    const auto asset = m_asset_manager->load_by_id_type_force_reload<Phos::PrefabAsset>(m_prefab_id);
    if (asset == nullptr) {
        PS_ERROR("[EditorPrefabHelper::ctor] Cannot open prefab, no prefab in path: {}", m_path.string());
        return;
    }

    m_prefab_entity = Phos::PrefabLoader::load(*asset, m_prefab_scene, m_asset_manager);
}

void EditorPrefabHelper::save() const {
    if (!std::filesystem::exists(m_path)) {
        PS_ERROR("[EditorPrefabHelper::save] Could not save prefab because path is not set");
        return;
    }

    save(m_path);
}

void EditorPrefabHelper::save(const std::filesystem::path& path) const {
    auto prefab_builder = AssetBuilder();

    prefab_builder.dump("assetType", "prefab");
    prefab_builder.dump("id", m_prefab_id);

    auto entity_builder = EntitySerializer::serialize(m_prefab_entity);
    prefab_builder.dump("components", entity_builder);

    std::ofstream file(path);
    file << prefab_builder;
}
