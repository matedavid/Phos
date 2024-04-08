#pragma once

#include <filesystem>
#include <memory>

#include "core/uuid.h"

namespace Phos {

// Forward declarations
class Scene;
class AssetManagerBase;

class Project {
  public:
    ~Project() = default;

    [[nodiscard]] static std::shared_ptr<Project> create(const std::filesystem::path& path);
    [[nodiscard]] static std::shared_ptr<Project> open(const std::filesystem::path& path);
    [[nodiscard]] static std::shared_ptr<Project> load(const std::filesystem::path& path);

    [[nodiscard]] std::string name() const { return m_name; }
    [[nodiscard]] UUID starting_scene_id() const { return m_starting_scene_id; }
    [[nodiscard]] std::shared_ptr<AssetManagerBase> asset_manager() const { return m_asset_manager; }

    [[nodiscard]] std::filesystem::path project_path() const { return m_project_path; }
    [[nodiscard]] std::filesystem::path assets_path() const { return m_assets_path; }
    [[nodiscard]] std::filesystem::path scripting_path() const { return m_scripting_path; }

  private:
    std::string m_name;

    std::filesystem::path m_project_path;
    std::filesystem::path m_assets_path;
    std::filesystem::path m_scripting_path;

    UUID m_starting_scene_id;
    std::shared_ptr<AssetManagerBase> m_asset_manager;

    Project(std::string name,
            std::filesystem::path project_path,
            std::filesystem::path assets_path,
            std::filesystem::path scripting_path,
            UUID starting_scene_id,
            std::shared_ptr<AssetManagerBase> asset_manager);
};

} // namespace Phos
