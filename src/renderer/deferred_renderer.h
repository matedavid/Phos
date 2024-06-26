#pragma once

#include <memory>
#include <vector>
#include <array>
#include <glm/glm.hpp>

#include "renderer/backend/renderer.h"
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
class StaticMesh;
class Material;
class Light;
class Cubemap;
class Event;
class Entity;

struct ModelInfoPushConstant {
    glm::mat4 model;
    glm::vec4 color;
};

class DeferredRenderer : public ISceneRenderer {
  public:
    explicit DeferredRenderer(std::shared_ptr<Scene> scene, SceneRendererConfig config);
    ~DeferredRenderer() override;

    void render(const std::shared_ptr<Camera>& camera) override;
    [[nodiscard]] std::shared_ptr<Texture> output_texture() const override;

    void change_config(const SceneRendererConfig& config) override;
    void set_scene(std::shared_ptr<Scene> scene) override;

    void window_resized(uint32_t width, uint32_t height) override;

  private:
    std::shared_ptr<Scene> m_scene;
    SceneRendererConfig m_config;

    std::vector<std::shared_ptr<CommandBuffer>> m_command_buffers;

    // Shadow mapping pass
    std::shared_ptr<Texture> m_directional_shadow_map_texture;
    std::shared_ptr<Framebuffer> m_directional_shadow_map_framebuffer;
    std::shared_ptr<Material> m_shadow_map_material;

    std::shared_ptr<GraphicsPipeline> m_directional_shadow_map_pipeline;
    std::shared_ptr<RenderPass> m_directional_shadow_map_pass;

    struct ShadowMappingPushConstants {
        glm::mat4 light_space_matrix;
        glm::mat4 model;
    };

    struct ShadowMappingInfo {
        std::array<glm::mat4, MAX_DIRECTIONAL_LIGHTS> light_space_matrices{};
        uint32_t number_directional_shadow_maps{};
    };
    std::shared_ptr<UniformBuffer> m_shadow_mapping_info;

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

    // Skybox pipeline
    std::shared_ptr<GraphicsPipeline> m_skybox_pipeline;
    bool m_skybox_enabled = true;

    // Skybox cube
    std::shared_ptr<StaticMesh> m_cube_mesh;
    std::shared_ptr<Material> m_cube_material;

    // Bloom pass
    std::shared_ptr<ComputePipeline> m_bloom_pipeline;
    std::shared_ptr<Texture> m_bloom_downsample_texture;
    std::shared_ptr<Texture> m_bloom_upsample_texture;

    void init(uint32_t width, uint32_t height);
    void init_shadow_map_pipeline(uint32_t shadow_map_resolution);
    void init_bloom_pipeline(const BloomConfig& config);
    void init_skybox_pipeline(const EnvironmentConfig& config);

    [[nodiscard]] std::vector<std::shared_ptr<Light>> get_light_info() const;

    struct RenderableEntity {
        glm::mat4 model;
        std::shared_ptr<StaticMesh> mesh;
        std::shared_ptr<Material> material;
    };
    [[nodiscard]] std::vector<RenderableEntity> get_renderable_entities() const;
};

} // namespace Phos
