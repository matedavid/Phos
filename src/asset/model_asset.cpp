#include "model_asset.h"

#include "scene/scene.h"
#include "scene/entity.h"

namespace Phos {

ModelAsset::ModelAsset(Node* parent) : m_parent(parent) {}

ModelAsset::~ModelAsset() {
    destroy_r(m_parent);
}

void ModelAsset::import_into_scene(std::shared_ptr<Scene>& scene) const {
    import_into_scene_r(scene, m_parent);
}

void ModelAsset::import_into_scene_r(std::shared_ptr<Scene>& scene, ModelAsset::Node* node) const {
    auto entity = scene->create_entity();

    if (node->mesh && node->material) {
        entity.add_component<MeshRendererComponent>();
        auto& mesh_renderer = entity.get_component<MeshRendererComponent>();

        mesh_renderer.mesh = node->mesh.value();
        mesh_renderer.material = node->material.value();
    } else if (node->mesh || node->material) {
        PS_WARNING("Incomplete MeshRendererComponent when importing model into scene, only mesh or material");
    }

    for (auto child : node->children)
        import_into_scene_r(scene, child);
}

void ModelAsset::destroy_r(Node* node) {
    for (auto* child : node->children) {
        destroy_r(child);
    }

    delete node;
}

} // namespace Phos