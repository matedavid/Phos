#pragma once

#include "core.h"

#include <memory>
#include <glm/glm.hpp>

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
class Skybox;
class Camera;
class Event;

struct LightsUniformBuffer {
    glm::vec4 positions[10];
    glm::vec4 colors[10];
    uint32_t count;
};

struct ModelInfoPushConstant {
    glm::mat4 model;
    glm::vec4 color;
};

class DeferredRenderer {
  public:
    DeferredRenderer();
    ~DeferredRenderer();

    void update();

  private:
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

    // Skybox pipeline
    std::shared_ptr<Skybox> m_skybox;
    std::shared_ptr<GraphicsPipeline> m_skybox_pipeline;

    // Blending pass
    std::shared_ptr<RenderPass> m_blending_pass;
    std::shared_ptr<GraphicsPipeline> m_blending_pipeline;

    // Models
    std::shared_ptr<Mesh> m_model;
    std::shared_ptr<Mesh> m_cube;

    // Frame information
    std::shared_ptr<Camera> m_camera;
    LightsUniformBuffer m_light_info{};

    glm::vec2 m_mouse_pos{};

    void update_light_info();
    void on_event(Event& event);
};

} // namespace Phos
