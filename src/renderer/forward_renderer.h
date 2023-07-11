#pragma once

#include "core.h"

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
class Skybox;
class Material;
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

class ForwardRenderer {
  public:
    ForwardRenderer();
    ~ForwardRenderer();

    void update();

  private:
    std::shared_ptr<CommandBuffer> m_command_buffer;

    // Rendering information
    std::shared_ptr<RenderPass> m_render_pass;
    std::shared_ptr<GraphicsPipeline> m_pbr_pipeline;

    std::shared_ptr<GraphicsPipeline> m_skybox_pipeline;
    std::shared_ptr<Skybox> m_skybox;

    // Models
    std::shared_ptr<Mesh> m_model;
    std::shared_ptr<Mesh> m_cube;

    std::shared_ptr<Material> m_cube_material;

    // Frame information
    std::shared_ptr<Camera> m_camera;
    LightsUniformBuffer m_light_info{};
    glm::vec2 m_mouse_pos{};

    void update_light_info();
    void on_event(Event& event);
};

} // namespace Phos
