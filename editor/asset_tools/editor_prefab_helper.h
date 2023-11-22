#pragma once

#include "core.h"
#include <filesystem>

#include "scene/scene.h"

namespace Phos {

// Forward declarations
class Scene;
class EditorAssetManager;

} // namespace Phos

class EditorPrefabHelper {
  public:
    ~EditorPrefabHelper() = default;

    [[nodiscard]] static std::shared_ptr<EditorPrefabHelper> create(
        std::shared_ptr<Phos::EditorAssetManager> asset_manager);
    [[nodiscard]] static std::shared_ptr<EditorPrefabHelper> open(
        const std::filesystem::path& path,
        std::shared_ptr<Phos::EditorAssetManager> asset_manager);

    void save() const;
    void save(const std::filesystem::path& path) const;

    [[nodiscard]] Phos::Entity get_entity() const { return m_prefab_entity; }

  private:
    std::shared_ptr<Phos::Scene> m_prefab_scene;
    Phos::Entity m_prefab_entity{};
    std::shared_ptr<Phos::EditorAssetManager> m_asset_manager;

    Phos::UUID m_prefab_id{};
    std::filesystem::path m_path;

    explicit EditorPrefabHelper(std::shared_ptr<Phos::EditorAssetManager> asset_manager);
    explicit EditorPrefabHelper(const std::filesystem::path& path,
                                std::shared_ptr<Phos::EditorAssetManager> asset_manager);
};
