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
class ComputePipeline;
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
    void render(const std::shared_ptr<Camera>& camera) override;
    [[nodiscard]] std::shared_ptr<Texture> output_texture() const override;

    void window_resized(uint32_t width, uint32_t height) override;

  private:
    std::shared_ptr<Scene> m_scene;
    std::shared_ptr<CommandBuffer> m_command_buffer;

    // Shadow mapping pass
    std::shared_ptr<Texture> m_shadow_map_texture;
    std::shared_ptr<Framebuffer> m_shadow_map_framebuffer;
    std::shared_ptr<Material> m_shadow_map_material;

    std::shared_ptr<GraphicsPipeline> m_shadow_map_pipeline;
    std::shared_ptr<RenderPass> m_shadow_map_pass;

    glm::mat4 m_light_space_matrix{};

    // Geometry pass
    std::shared_ptr<Texture> m_position_texture;
    std::shared_ptr<Texture> m_normal_texture;
    std::shared_ptr<Texture> m_albedo_texture;
    std::shared_ptr<Texture> m_metallic_roughness_ao_texture;
    std::shared_ptr<Texture> m_emission_texture;

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

    // Tone mapping Pass
    std::shared_ptr<Texture> m_tone_mapping_texture;
    std::shared_ptr<Framebuffer> m_tone_mapping_framebuffer;

    std::shared_ptr<GraphicsPipeline> m_tone_mapping_pipeline;
    std::shared_ptr<RenderPass> m_tone_mapping_pass;

    // Cubemap pipeline
    std::shared_ptr<Cubemap> m_skybox;
    std::shared_ptr<GraphicsPipeline> m_skybox_pipeline;

    // Skybox cube
    std::shared_ptr<Mesh> m_cube_mesh;
    std::shared_ptr<Material> m_cube_material;

    // Compute test
    std::shared_ptr<ComputePipeline> m_compute_pipeline;
    std::shared_ptr<Texture> m_compute_output_texture;

    void init();
    [[nodiscard]] std::vector<std::shared_ptr<Light>> get_light_info() const;
};

} // namespace Phos
