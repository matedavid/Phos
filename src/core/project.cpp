#include "project.h"

#include <yaml-cpp/yaml.h>

#include "utility/logging.h"
#include "asset/editor_asset_manager.h"
#include "scene/scene.h"

namespace Phos {

Project::Project(std::string name,
                 std::filesystem::path path,
                 std::shared_ptr<Scene> scene,
                 std::shared_ptr<AssetManagerBase> asset_manager)
      : m_name(std::move(name)), m_path(std::move(path)), m_scene(std::move(scene)),
        m_asset_manager(std::move(asset_manager)) {}

std::shared_ptr<Project> Project::create(const std::filesystem::path& path) {
    (void)path;
    PHOS_LOG_ERROR("Unimplemented");
    return nullptr;
}

std::shared_ptr<Project> Project::open(const std::filesystem::path& path) {
    const auto containing_folder = std::filesystem::path(path).parent_path();

    const auto node = YAML::LoadFile(path);
    const auto project_name = node["name"].as<std::string>();

    const auto asset_manager = std::make_shared<EditorAssetManager>(containing_folder);

    std::vector<std::shared_ptr<Scene>> scenes;
    for (uint32_t i = 0; i < node["scenes"].size(); ++i) {
        const auto scene_id = UUID(node["scenes"][i].as<uint64_t>());

        const auto scene = asset_manager->load_by_id_type<Scene>(scene_id);
        scenes.push_back(scene);
    }

    // TODO: Should pass all of the scenes in the project file, at the moment only one is allowed
    auto* project = new Project(project_name, std::filesystem::absolute(path.parent_path()), scenes[0], asset_manager);
    return std::shared_ptr<Project>(project);
}

std::shared_ptr<Project> Project::load(const std::filesystem::path& path) {
    (void)path;
    PHOS_LOG_ERROR("Unimplemented");
    return nullptr;
}

} // namespace Phos
