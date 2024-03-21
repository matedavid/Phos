#include "project.h"

#include <yaml-cpp/yaml.h>

#include "utility/logging.h"
#include "asset/editor_asset_manager.h"
#include "scene/scene.h"

namespace Phos {

Project::Project(std::string name,
                 std::filesystem::path project_path,
                 std::filesystem::path assets_path,
                 std::filesystem::path scripting_path,
                 UUID starting_scene_id,
                 std::shared_ptr<AssetManagerBase> asset_manager)
      : m_name(std::move(name)), m_project_path(std::move(project_path)), m_assets_path(std::move(assets_path)),
        m_scripting_path(std::move(scripting_path)), m_starting_scene_id(starting_scene_id),
        m_asset_manager(std::move(asset_manager)) {}

std::shared_ptr<Project> Project::create(const std::filesystem::path& path) {
    (void)path;
    PHOS_LOG_ERROR("Unimplemented");
    return nullptr;
}

std::shared_ptr<Project> Project::open(const std::filesystem::path& path) {
    const auto node = YAML::LoadFile(path);
    const auto project_name = node["name"].as<std::string>();

    const auto project_path = std::filesystem::path(path).parent_path();
    const auto assets_path = project_path / "Assets";
    const auto scripting_path = project_path / "bin" / "Debug" / (project_name + ".dll");

    const auto asset_manager = std::make_shared<EditorAssetManager>(assets_path);

    std::vector<std::shared_ptr<Scene>> scenes;
    const auto starting_scene_id = UUID(node["startingScene"].as<uint64_t>());

    // TODO: Should pass all of the scenes in the project file, at the moment only one is allowed
    auto* project =
        new Project(project_name, project_path, assets_path, scripting_path, starting_scene_id, asset_manager);
    return std::shared_ptr<Project>(project);
}

std::shared_ptr<Project> Project::load(const std::filesystem::path& path) {
    (void)path;
    PHOS_LOG_ERROR("Unimplemented");
    return nullptr;
}

} // namespace Phos
