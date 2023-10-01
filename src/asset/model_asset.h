#pragma once

#include "core.h"

#include <optional>

#include "asset/asset.h"

namespace Phos {

// Forward declarations
class Mesh;
class Material;
class Scene;

class ModelAsset : public IAsset {
  public:
    struct Node {
        // std::optional<std::shared_ptr<Mesh>> mesh;
        // std::optional<std::shared_ptr<Material>> material;

        std::optional<UUID> mesh_id;
        std::optional<UUID> material_id;

        std::vector<Node*> children;
    };

    explicit ModelAsset(Node* parent);
    virtual ~ModelAsset();

    void import_into_scene(std::shared_ptr<Scene>& scene) const;

    [[nodiscard]] AssetType asset_type() override { return AssetType::Model; }

  private:
    Node* m_parent;

    UUID import_into_scene_r(std::shared_ptr<Scene>& scene, Node* node, UUID* parent_uuid) const;
    void destroy_r(Node* node);
};

} // namespace Phos
