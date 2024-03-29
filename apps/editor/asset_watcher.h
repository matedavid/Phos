#pragma once

#include "asset/asset.h"
#include <filesystem>

namespace Phos {

// Forward declarations
class Scene;
class Project;
class EditorAssetManager;
class Entity;
class ISceneRenderer;
class ScriptingSystem;

class Cubemap;
class Material;
class Mesh;

} // namespace Phos

class AssetWatcher {
  public:
    AssetWatcher(std::shared_ptr<Phos::Scene> scene,
                 std::shared_ptr<Phos::Project> project,
                 std::shared_ptr<Phos::ISceneRenderer> renderer,
                 std::shared_ptr<Phos::ScriptingSystem> scripting);
    ~AssetWatcher() = default;

    void check_asset_modified();

    void asset_created(const std::filesystem::path& path);
    void asset_renamed(const std::filesystem::path& old_path, const std::filesystem::path& new_path);
    void asset_removed(const std::filesystem::path& path);

  private:
    std::shared_ptr<Phos::Scene> m_scene;
    std::shared_ptr<Phos::Project> m_project;
    std::shared_ptr<Phos::EditorAssetManager> m_asset_manager;
    std::shared_ptr<Phos::ISceneRenderer> m_renderer;
    std::shared_ptr<Phos::ScriptingSystem> m_scripting;

    std::filesystem::path m_dll_path;
    std::unordered_map<std::filesystem::path, uint64_t> m_watching;
    std::unordered_map<std::filesystem::path, std::pair<Phos::UUID, Phos::AssetType>> m_path_to_info;

    void add_file(const std::filesystem::path& path);
    void add_directory(const std::filesystem::path& path);

    [[nodiscard]] static bool is_watchable_asset_type(Phos::AssetType type);
    void start_watching_asset(const std::filesystem::path& path, Phos::AssetType type, Phos::UUID id);

    void update_cubemap(const std::shared_ptr<Phos::Cubemap>& cubemap) const;
    void update_material(const std::shared_ptr<Phos::Material>& mat) const;
    void update_mesh(const std::shared_ptr<Phos::Mesh>& mesh) const;
    void update_script();

    bool m_script_update_pending = false;
};
