#pragma once

#include "asset/asset.h"

#include "scene/registry.h"
#include "scene/components.h"
#include "scene/scene_renderer.h"

namespace Phos {

// Forward declarations
class Camera;
class Entity;

class Scene : public IAsset {
  public:
    explicit Scene(std::string name);
    Scene(Scene& other);
    ~Scene() override;

    [[nodiscard]] AssetType asset_type() override { return AssetType::Scene; }

    Entity create_entity();
    Entity create_entity(UUID uuid);
    Entity create_entity(const std::string& name, UUID uuid);

    void destroy_entity(Entity entity);

    [[nodiscard]] Entity get_entity_with_uuid(const UUID& uuid);
    [[nodiscard]] std::vector<Entity> get_all_entities() const;

    template <typename... Components>
    [[nodiscard]] std::vector<Entity> get_entities_with();

    [[nodiscard]] std::string name() const { return m_name; }
    [[nodiscard]] SceneRendererConfig& config() { return m_renderer_config; }

  private:
    std::string m_name;
    std::unique_ptr<Registry> m_registry;
    SceneRendererConfig m_renderer_config;

    std::unordered_map<std::size_t, Entity*> m_id_to_entity;
    std::unordered_map<UUID, Entity*> m_uuid_to_entity;

    void destroy_entity_r(Entity entity);

    friend class Entity;
};

} // namespace Phos
