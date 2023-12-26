#pragma once

#include <filesystem>
#include <memory>

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
    [[nodiscard]] std::shared_ptr<Scene> scene() const { return m_scene; }
    [[nodiscard]] std::shared_ptr<AssetManagerBase> asset_manager() const { return m_asset_manager; }
    [[nodiscard]] std::filesystem::path path() const { return m_path; }

  private:
    std::string m_name;
    std::filesystem::path m_path;

    std::shared_ptr<Scene> m_scene;
    std::shared_ptr<AssetManagerBase> m_asset_manager;

    Project(std::string name,
            std::filesystem::path path,
            std::shared_ptr<Scene> scene,
            std::shared_ptr<AssetManagerBase> asset_manager);
};

} // namespace Phos
