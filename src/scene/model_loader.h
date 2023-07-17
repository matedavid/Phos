#pragma once

#include "core.h"

// Forward declarations
struct aiNode;
struct aiScene;
struct aiMesh;

namespace Phos {

// Forward declarations
class Scene;
class Entity;
class Mesh;
class Material;

class ModelLoader {
  public:
    ModelLoader() = delete;
    ~ModelLoader() = delete;

    static Entity load_into_scene(const std::string& path, const std::shared_ptr<Scene>& scene, bool flip_uvs = false);

  private:
    static void process_node_r(const aiNode* node,
                               const aiScene* loaded_scene,
                               const std::shared_ptr<Scene>& scene,
                               const std::string& directory,
                               Entity parent_entity);

    static std::shared_ptr<Mesh> load_mesh(const aiMesh* mesh);
    static std::shared_ptr<Material> load_material(const aiMesh* mesh,
                                                   const aiScene* scene,
                                                   const std::string& directory);
};

} // namespace Phos
