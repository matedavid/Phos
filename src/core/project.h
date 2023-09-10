#pragma once

#include "core.h"

namespace Phos {

// Forward declarations
class Scene;
class AssetManagerBase;

class Project {
  public:
    ~Project() = default;

    [[nodiscard]] static std::shared_ptr<Project> create(const std::string& path);
    [[nodiscard]] static std::shared_ptr<Project> open(const std::string& path);
    [[nodiscard]] static std::shared_ptr<Project> load(const std::string& path);

    [[nodiscard]] std::string name() const { return m_name; }
    [[nodiscard]] std::shared_ptr<Scene> scene() const { return m_scene; }
    [[nodiscard]] std::shared_ptr<AssetManagerBase> asset_manager() const { return m_asset_manager; }

  private:
    std::string m_name;

    std::shared_ptr<Scene> m_scene;
    std::shared_ptr<AssetManagerBase> m_asset_manager;

    Project(std::shared_ptr<Scene> scene, std::shared_ptr<AssetManagerBase> asset_manager);
};

} // namespace Phos
