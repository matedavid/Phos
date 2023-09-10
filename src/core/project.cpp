#include "project.h"

#include <yaml-cpp/yaml.h>
#include <filesystem>

#include "asset/editor_asset_manager.h"
#include "scene/scene.h"
#include "scene/scene_deserializer.h"

namespace Phos {

Project::Project(std::shared_ptr<Scene> scene, std::shared_ptr<AssetManagerBase> asset_manager)
      : m_scene(std::move(scene)), m_asset_manager(std::move(asset_manager)) {}

std::shared_ptr<Project> Project::create(const std::string& path) {
    PS_FAIL("Unimplemented");
    return nullptr;
}

std::shared_ptr<Project> Project::open(const std::string& path) {
    const auto containing_folder = std::filesystem::path(path).parent_path();

    const auto node = YAML::LoadFile(path);
    // const auto project_name = node["name"].as<std::string>();

    const auto asset_manager = std::make_shared<EditorAssetManager>(containing_folder);

    std::vector<std::shared_ptr<Scene>> scenes;
    for (uint32_t i = 0; i < node["scenes"].size(); ++i) {
        const auto scene_path = containing_folder / node["scenes"][i].as<std::string>();

        const auto scene = SceneDeserializer::deserialize(scene_path, asset_manager);
        scenes.push_back(scene);
    }

    // TODO: Should pass all of the scenes in the project file, at the moment only one is allowed
    auto* project = new Project(scenes[0], asset_manager);
    return std::shared_ptr<Project>(project);
}

std::shared_ptr<Project> Project::load(const std::string& path) {
    PS_FAIL("Unimplemented")
}

} // namespace Phos
