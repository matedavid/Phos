#pragma once

#include "core.h"

#include <memory>
#include <glm/glm.hpp>

#include "scene/scene_renderer.h"

namespace Phos {

// Forward declarations
class CommandBuffer;
class Texture;
class Framebuffer;
class GraphicsPipeline;
class RenderPass;
class VertexBuffer;
class IndexBuffer;
class UniformBuffer;
class Mesh;
class Material;
class Light;
class Cubemap;
class Event;

struct ModelInfoPushConstant {
    glm::mat4 model;
    glm::vec4 color;
};

class DeferredRenderer : public ISceneRenderer {
  public:
    explicit DeferredRenderer(std::shared_ptr<Scene> scene);
    ~DeferredRenderer() override;

    void set_scene(std::shared_ptr<Scene> scene) override;
    void render() override;

  private:
    std::shared_ptr<Scene> m_scene;

    std::shared_ptr<CommandBuffer> m_command_buffer;

    // Geometry pass
    std::shared_ptr<Texture> m_position_texture;
    std::shared_ptr<Texture> m_normal_texture;
    std::shared_ptr<Texture> m_albedo_texture;
    std::shared_ptr<Texture> m_metallic_roughness_ao_texture;

    std::shared_ptr<Framebuffer> m_geometry_framebuffer;

    std::shared_ptr<GraphicsPipeline> m_geometry_pipeline;
    std::shared_ptr<RenderPass> m_geometry_pass;

    // Lighting pass
    std::shared_ptr<VertexBuffer> m_quad_vertex;
    std::shared_ptr<IndexBuffer> m_quad_index;

    std::shared_ptr<Texture> m_lighting_texture;
    std::shared_ptr<Framebuffer> m_lighting_framebuffer;

    std::shared_ptr<GraphicsPipeline> m_lighting_pipeline;
    std::shared_ptr<RenderPass> m_lighting_pass;

    // Cubemap pipeline
    std::shared_ptr<Cubemap> m_skybox;
    std::shared_ptr<GraphicsPipeline> m_skybox_pipeline;

    // Blending pass
    std::shared_ptr<RenderPass> m_blending_pass;
    std::shared_ptr<GraphicsPipeline> m_blending_pipeline;

    // Skybox cube
    std::shared_ptr<Mesh> m_cube_mesh;
    std::shared_ptr<Material> m_cube_material;

    [[nodiscard]] std::vector<std::shared_ptr<Light>> get_light_info() const;
};

} // namespace Phos
