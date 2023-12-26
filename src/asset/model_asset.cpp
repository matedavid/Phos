#include "model_asset.h"

#include "utility/logging.h"
#include "scene/scene.h"

namespace Phos {

ModelAsset::ModelAsset(Node* parent) : m_parent(parent) {}

ModelAsset::~ModelAsset() {
    destroy_r(m_parent);
}

void ModelAsset::import_into_scene(std::shared_ptr<Scene>& scene) const {
    import_into_scene_r(scene, m_parent, nullptr);
}

UUID ModelAsset::import_into_scene_r(std::shared_ptr<Scene>& scene, ModelAsset::Node* node, UUID* parent_uuid) const {
    auto entity = scene->create_entity();
    auto uuid = entity.get_component<Phos::UUIDComponent>().uuid;

    if (node->mesh && node->material) {
        entity.add_component<MeshRendererComponent>();
        auto& mesh_renderer = entity.get_component<MeshRendererComponent>();

        mesh_renderer.mesh = node->mesh.value();
        mesh_renderer.material = node->material.value();
    } else if (node->mesh || node->material) {
        PHOS_LOG_WARNING("Incomplete MeshRendererComponent when importing model into scene, only mesh or material");
    }

    auto& relationship = entity.get_component<RelationshipComponent>();
    if (parent_uuid != nullptr)
        relationship.parent = *parent_uuid;

    for (const auto child : node->children) {
        auto child_uuid = import_into_scene_r(scene, child, &uuid);
        relationship.children.push_back(child_uuid);
    }

    return uuid;
}

void ModelAsset::destroy_r(const Node* node) {
    for (const auto* child : node->children) {
        destroy_r(child);
    }

    delete node;
}

} // namespace Phos