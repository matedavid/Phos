#include "deferred_renderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "core/window.h"

#include "managers/shader_manager.h"

#include "scene/scene.h"
#include "scene/entity.h"
#include "scene/model_loader.h"

#include "renderer/mesh.h"
#include "renderer/camera.h"
#include "renderer/light.h"

#include "renderer/backend/renderer.h"
#include "renderer/backend/command_buffer.h"
#include "renderer/backend/texture.h"
#include "renderer/backend/image.h"
#include "renderer/backend/framebuffer.h"
#include "renderer/backend/shader.h"
#include "renderer/backend/graphics_pipeline.h"
#include "renderer/backend/compute_pipeline.h"
#include "renderer/backend/render_pass.h"
#include "renderer/backend/buffers.h"
#include "renderer/backend/material.h"
#include "renderer/backend/cubemap.h"

namespace Phos {

DeferredRenderer::DeferredRenderer(std::shared_ptr<Scene> scene) : m_scene(std::move(scene)) {
    m_command_buffer = CommandBuffer::create();

    m_shadow_map_material =
        Material::create(Renderer::shader_manager()->get_builtin_shader("ShadowMap"), "ShadowMap Material");
    PS_ASSERT(m_shadow_map_material->bake(), "Failed to bake Shadow Map material")

    const auto faces = Cubemap::Faces{
        .right = "right.jpg",
        .left = "left.jpg",
        .top = "top.jpg",
        .bottom = "bottom.jpg",
        .front = "front.jpg",
        .back = "back.jpg",
    };
    m_skybox = Cubemap::create(faces, "../assets/skybox/");

    // Create cube
    m_cube_mesh = ModelLoader::load_single_mesh("../assets/cube.fbx");
    m_cube_material = Material::create(Renderer::shader_manager()->get_builtin_shader("Skybox"), "SkyboxMaterial");
    m_cube_material->bake();

    init(Renderer::config().window->get_width(), Renderer::config().window->get_height());
}

DeferredRenderer::~DeferredRenderer() {
    Renderer::wait_idle();
}

void DeferredRenderer::set_scene(std::shared_ptr<Scene> scene) {
    (void)scene;
    PS_FAIL("Unimplemented")
}

struct ShadowMappingPushConstants {
    glm::mat4 light_space_matrix;
    glm::mat4 model;
};

void DeferredRenderer::render(const std::shared_ptr<Camera>& camera) {
    const FrameInformation frame_info = {
        .camera = camera,
        .lights = get_light_info(),
    };

    Renderer::begin_frame(frame_info);

    m_command_buffer->record([&]() {
        // ShadowMapping pass
        // ==================
        {
            Renderer::begin_render_pass(m_command_buffer, m_shadow_map_pass);

            const auto it = std::ranges::find_if(frame_info.lights, [](const std::shared_ptr<Light>& light) {
                return light->type() == Light::Type::Directional && light->shadow_type != Light::ShadowType::None;
            });

            if (it != frame_info.lights.end()) {
                const auto directional_light = std::dynamic_pointer_cast<DirectionalLight>(*it);

                Renderer::bind_graphics_pipeline(m_command_buffer, m_shadow_map_pipeline);

                // Prepare light space matrix
                constexpr float znear = 0.01f, zfar = 40.0f;
                constexpr float size = 10.0f;
                const auto light_view = glm::lookAt(directional_light->position,
                                                    directional_light->position + directional_light->direction,
                                                    glm::vec3(0.0f, 1.0f, 0.0f));
                const auto light_projection = glm::ortho(-size, size, size, -size, znear, zfar);

                m_light_space_matrix = light_projection * light_view;

                // Render models
                for (const auto& entity : get_renderable_entities()) {
                    const auto& mr_component = entity.get_component<MeshRendererComponent>();
                    const auto& transform = entity.get_component<TransformComponent>();

                    glm::mat4 model{1.0f};
                    model = glm::translate(model, transform.position);
                    model = glm::rotate(model, transform.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
                    model = glm::rotate(model, transform.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
                    model = glm::rotate(model, transform.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
                    model = glm::scale(model, transform.scale);

                    auto constants = ShadowMappingPushConstants{
                        .light_space_matrix = m_light_space_matrix,
                        .model = model,
                    };

                    Renderer::bind_push_constant(m_command_buffer, m_shadow_map_pipeline, constants);
                    Renderer::submit_static_mesh(m_command_buffer, mr_component.mesh, m_shadow_map_material);
                }
            }

            Renderer::end_render_pass(m_command_buffer, m_shadow_map_pass);
        }

        // Geometry pass
        // =============
        {
            Renderer::begin_render_pass(m_command_buffer, m_geometry_pass);

            // Draw model
            Renderer::bind_graphics_pipeline(m_command_buffer, m_geometry_pipeline);

            for (const auto& entity : get_renderable_entities()) {
                const auto& mr_component = entity.get_component<MeshRendererComponent>();
                const auto& transform = entity.get_component<TransformComponent>();

                glm::mat4 model{1.0f};
                model = glm::translate(model, transform.position);
                model = glm::rotate(model, transform.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
                model = glm::rotate(model, transform.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::rotate(model, transform.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
                model = glm::scale(model, transform.scale);

                auto constants = ModelInfoPushConstant{
                    .model = model,
                    .color = glm::vec4(1.0f),
                };

                Renderer::bind_push_constant(m_command_buffer, m_geometry_pipeline, constants);
                Renderer::submit_static_mesh(m_command_buffer, mr_component.mesh, mr_component.material);
            }

            Renderer::end_render_pass(m_command_buffer, m_geometry_pass);
        }

        // Lighting pass
        // ==========================
        {
            Renderer::begin_render_pass(m_command_buffer, m_lighting_pass);

            // Draw quad
            Renderer::bind_graphics_pipeline(m_command_buffer, m_lighting_pipeline);

            auto lighting_constants = ShadowMappingPushConstants{
                .light_space_matrix = m_light_space_matrix,
                .model = glm::mat4(1.0f),
            };
            Renderer::bind_push_constant(m_command_buffer, m_lighting_pipeline, lighting_constants);

            Renderer::draw_screen_quad(m_command_buffer);

            // Draw skybox
            Renderer::bind_graphics_pipeline(m_command_buffer, m_skybox_pipeline);

            glm::mat4 model{1.0f};
            model = glm::scale(model, glm::vec3(1.0f));

            const auto constants = ModelInfoPushConstant{
                .model = model,
                .color = glm::vec4{1.0f},
            };
            Renderer::bind_push_constant(m_command_buffer, m_skybox_pipeline, constants);
            Renderer::submit_static_mesh(m_command_buffer, m_cube_mesh, m_cube_material);

            Renderer::end_render_pass(m_command_buffer, m_lighting_pass);
        }

        // Bloom pass
        // ==========================
        m_bloom_pipeline->execute(command_buffer);

        // Tone Mapping pass
        // ==========================
        {
            Renderer::begin_render_pass(m_command_buffer, m_tone_mapping_pass);

            Renderer::bind_graphics_pipeline(m_command_buffer, m_tone_mapping_pipeline);
            Renderer::draw_screen_quad(m_command_buffer);

            Renderer::end_render_pass(m_command_buffer, m_tone_mapping_pass);
        }
    });

    // Submit command buffer
    Renderer::submit_command_buffer(m_command_buffer);

    Renderer::end_frame();

    // @TODO: Should rethink
    //    Renderer::wait_idle();
    //    m_bloom_pipeline->invalidate();
}

std::shared_ptr<Texture> DeferredRenderer::output_texture() const {
    return m_tone_mapping_texture;
}

void DeferredRenderer::window_resized(uint32_t width, uint32_t height) {
    Renderer::wait_idle();

    init(width, height);
}

void DeferredRenderer::init(uint32_t width, uint32_t height) {
    const Image::Description depth_image_description = {
        .width = width,
        .height = height,
        .type = Image::Type::Image2D,
        .format = Image::Format::D32_SFLOAT,
        .transfer = false,
        .attachment = true,
    };
    const auto depth_image = Image::create(depth_image_description);

    // Shadow mapping pass
    {
        m_shadow_map_texture = Texture::create(Image::create({
            .width = width,
            .height = height,
            .type = Image::Type::Image2D,
            .format = Image::Format::D32_SFLOAT,
            .transfer = false,
            .attachment = true,
        }));

        const auto shadow_depth_attachment = Framebuffer::Attachment{
            .image = m_shadow_map_texture->get_image(),
            .load_operation = LoadOperation::Clear,
            .store_operation = StoreOperation::Store,
            .clear_value = glm::vec3(1.0f),

            .input_depth = true,
        };

        m_shadow_map_framebuffer = Framebuffer::create({
            .attachments = {shadow_depth_attachment},
        });

        m_shadow_map_pipeline = GraphicsPipeline::create({
            .shader = Renderer::shader_manager()->get_builtin_shader("ShadowMap"),
            .target_framebuffer = m_shadow_map_framebuffer,
            .depth_write = true,
        });

        m_shadow_map_pass = RenderPass::create({
            .debug_name = "Shadow Mapping pass",
            .target_framebuffer = m_shadow_map_framebuffer,
        });
    }

    // Geometry pass
    {
        m_position_texture = Texture::create(Image::create({
            .width = width,
            .height = height,
            .type = Image::Type::Image2D,
            .format = Image::Format::R16G16B16A16_SFLOAT,
            .attachment = true,
        }));

        m_normal_texture = Texture::create(Image::create({
            .width = width,
            .height = height,
            .type = Image::Type::Image2D,
            .format = Image::Format::R16G16B16A16_SFLOAT,
            .attachment = true,
        }));

        m_albedo_texture = Texture::create(Image::create({
            .width = width,
            .height = height,
            .type = Image::Type::Image2D,
            .format = Image::Format::R16G16B16A16_SFLOAT,
            .attachment = true,
        }));

        m_metallic_roughness_ao_texture = Texture::create(width, height);

        m_emission_texture = Texture::create(Image::create({
            .width = width,
            .height = height,
            .type = Image::Type::Image2D,
            .format = Image::Format::R16G16B16A16_SFLOAT,
            .attachment = true,
        }));

        const auto position_attachment = Framebuffer::Attachment{
            .image = m_position_texture->get_image(),
            .load_operation = LoadOperation::Clear,
            .store_operation = StoreOperation::Store,
            .clear_value = glm::vec3(0.0f),
        };
        const auto normal_attachment = Framebuffer::Attachment{
            .image = m_normal_texture->get_image(),
            .load_operation = LoadOperation::Clear,
            .store_operation = StoreOperation::Store,
            .clear_value = glm::vec3(0.0f),
        };
        const auto albedo_attachment = Framebuffer::Attachment{
            .image = m_albedo_texture->get_image(),
            .load_operation = LoadOperation::Clear,
            .store_operation = StoreOperation::Store,
            .clear_value = glm::vec3(0.0f),
        };
        const auto metallic_roughness_ao_attachment = Framebuffer::Attachment{
            .image = m_metallic_roughness_ao_texture->get_image(),
            .load_operation = LoadOperation::Clear,
            .store_operation = StoreOperation::Store,
            .clear_value = glm::vec3(0.0f),
        };
        const auto emission_attachment = Framebuffer::Attachment{
            .image = m_emission_texture->get_image(),
            .load_operation = LoadOperation::Clear,
            .store_operation = StoreOperation::Store,
            .clear_value = glm::vec3(0.0f),
        };
        const auto depth_attachment = Framebuffer::Attachment{
            .image = depth_image,
            .load_operation = LoadOperation::Clear,
            .store_operation = StoreOperation::Store,
            .clear_value = glm::vec3(1.0f),
        };

        m_geometry_framebuffer = Framebuffer::create(Framebuffer::Description{
            .attachments = {position_attachment,
                            normal_attachment,
                            albedo_attachment,
                            metallic_roughness_ao_attachment,
                            emission_attachment,
                            depth_attachment},
        });

        m_geometry_pipeline = GraphicsPipeline::create(GraphicsPipeline::Description{
            .shader = Renderer::shader_manager()->get_builtin_shader("PBR.Geometry.Deferred"),
            .target_framebuffer = m_geometry_framebuffer,
        });

        m_geometry_pass = RenderPass::create(RenderPass::Description{
            .debug_name = "Deferred-Geometry",
            .target_framebuffer = m_geometry_framebuffer,
        });
    }

    // Lighting pass
    {
        m_lighting_texture = Texture::create(Image::create({
            .width = width,
            .height = height,
            .type = Image::Type::Image2D,
            .format = Image::Format::R16G16B16A16_SFLOAT,
            .attachment = true,
            .storage = true,
        }));

        const auto lighting_attachment = Framebuffer::Attachment{
            .image = m_lighting_texture->get_image(),
            .load_operation = LoadOperation::Clear,
            .store_operation = StoreOperation::Store,
            .clear_value = glm::vec3(0.0f),
        };

        const auto depth_attachment = Framebuffer::Attachment{
            .image = depth_image,
            .load_operation = LoadOperation::Load,
            .store_operation = StoreOperation::DontCare,
            .clear_value = glm::vec3(1.0f),
        };
        m_lighting_framebuffer = Framebuffer::create({.attachments = {lighting_attachment, depth_attachment}});

        m_lighting_pipeline = GraphicsPipeline::create(GraphicsPipeline::Description{
            .shader = Renderer::shader_manager()->get_builtin_shader("PBR.Lighting.Deferred"),
            .target_framebuffer = m_lighting_framebuffer,

            .depth_write = false,
        });

        m_lighting_pipeline->add_input("uPositionMap", m_position_texture);
        m_lighting_pipeline->add_input("uNormalMap", m_normal_texture);
        m_lighting_pipeline->add_input("uAlbedoMap", m_albedo_texture);
        m_lighting_pipeline->add_input("uMetallicRoughnessAOMap", m_metallic_roughness_ao_texture);
        m_lighting_pipeline->add_input("uEmissionMap", m_emission_texture);
        m_lighting_pipeline->add_input("uShadowMap", m_shadow_map_texture);
        PS_ASSERT(m_geometry_pipeline->bake(), "Failed to bake Lighting Pipeline")

        m_lighting_pass = RenderPass::create(RenderPass::Description{
            .debug_name = "Deferred-Lighting",
            .target_framebuffer = m_lighting_framebuffer,
        });
    }

    // Cubemap pass
    {
        m_skybox_pipeline = GraphicsPipeline::create(GraphicsPipeline::Description{
            .shader = Renderer::shader_manager()->get_builtin_shader("Skybox"),
            .target_framebuffer = m_lighting_framebuffer,

            .front_face = FrontFace::Clockwise,
            .depth_compare_op = DepthCompareOp::LessEq,
        });

        m_skybox_pipeline->add_input("uSkybox", m_skybox);
        PS_ASSERT(m_skybox_pipeline->bake(), "Failed to bake Cubemap Pipeline")
    }

    // Bloom pass
    {
        m_bloom_downsample_texture = Texture::create(Image::create({
            .width = width,
            .height = height,
            .type = Image::Type::Image2D,
            .format = Image::Format::R16G16B16A16_SFLOAT,
            .generate_mips = true,
            .attachment = false,
            .storage = true,
        }));

        m_bloom_upsample_texture = Texture::create(Image::create({
            .width = width,
            .height = height,
            .type = Image::Type::Image2D,
            .format = Image::Format::R16G16B16A16_SFLOAT,
            .generate_mips = true,
            .attachment = false,
            .storage = true,
        }));

        m_bloom_pipeline = ComputePipeline::create(ComputePipeline::Description{
            .shader = Shader::create("../shaders/build/Bloom.Compute.spv"),
        });

        const auto get_mip_size = [](const std::shared_ptr<Texture>& tex, uint32_t level) -> glm::uvec2 {
            const auto& img = tex->get_image();

            PS_ASSERT(level < img->num_mips(), "Mip level not valid")

            const auto img_width = img->width();
            const auto img_height = img->height();

            const auto mip_width = (uint32_t)std::round((float)img_width / std::pow(2.0f, (float)level));
            const auto mip_height = (uint32_t)std::round((float)img_height / std::pow(2.0f, (float)level));

            return {mip_width, mip_height};
        };

        constexpr int32_t MODE_DOWNSAMPLE = 0;
        constexpr int32_t MODE_UPSAMPLE = 1;
        constexpr int32_t MODE_PREFILTER = 2;

        struct BloomPushConstants {
            int32_t mode;
            float threshold;
        };

        auto bloom_constants = BloomPushConstants{};
        bloom_constants.threshold = 1.0f;

        // Prefilter step
        bloom_constants.mode = MODE_PREFILTER;
        auto work_groups = get_mip_size(m_bloom_downsample_texture, 1) / 2u;
        m_bloom_pipeline->add_step(
            [&](ComputePipeline::StepBuilder& builder) {
                builder.set("uInputImage", m_lighting_texture);
                builder.set("uOutputImage", m_bloom_downsample_texture, 1);
                builder.set_push_constants("uInfo", bloom_constants);
            },
            {work_groups, 1});

        // Down sampling
        constexpr uint32_t downsampling_range = 5;
        const auto num_mips = m_bloom_downsample_texture->get_image()->num_mips();

        bloom_constants.mode = MODE_DOWNSAMPLE;

        for (uint32_t i = 2; i < num_mips - downsampling_range; ++i) {
            work_groups = get_mip_size(m_bloom_downsample_texture, i) / 2u;

            m_bloom_pipeline->add_step(
                [&](ComputePipeline::StepBuilder& builder) {
                    builder.set("uInputImage", m_bloom_downsample_texture, i - 1);
                    builder.set("uOutputImage", m_bloom_downsample_texture, i);
                    builder.set_push_constants("uInfo", bloom_constants);
                },
                {work_groups, 1});
        }

        // Up sampling
        bloom_constants.mode = MODE_UPSAMPLE;

        const auto last_mip = num_mips - (downsampling_range + 1);
        for (uint32_t i = last_mip; i > 0; --i) {
            work_groups = get_mip_size(m_bloom_upsample_texture, i - 1) / 2u;

            m_bloom_pipeline->add_step(
                [&](ComputePipeline::StepBuilder& builder) {
                    builder.set("uInputImage", m_bloom_downsample_texture, i);
                    builder.set("uOutputImage", m_bloom_upsample_texture, i - 1);
                    builder.set_push_constants("uInfo", bloom_constants);
                },
                {work_groups, 1});
        }
    }

    // Tone Mapping pass
    {
        m_tone_mapping_texture = Texture::create(Image::create({
            .width = width,
            .height = height,
            .type = Image::Type::Image2D,
            .format = Image::Format::B8G8R8A8_SRGB,
            .attachment = true,
        }));

        const auto tone_mapping_attachment = Framebuffer::Attachment{
            .image = m_tone_mapping_texture->get_image(),
            .load_operation = LoadOperation::Clear,
            .store_operation = StoreOperation::Store,
            .clear_value = glm::vec3(0.0f),
        };

        m_tone_mapping_framebuffer = Framebuffer::create({
            .attachments = {tone_mapping_attachment},
        });

        m_tone_mapping_pipeline = GraphicsPipeline::create({
            .shader = Renderer::shader_manager()->get_builtin_shader("ToneMapping"),
            .target_framebuffer = m_tone_mapping_framebuffer,
        });

        m_tone_mapping_pipeline->add_input("uResultTexture", m_lighting_texture);
        m_tone_mapping_pipeline->add_input("uBloomTexture", m_bloom_upsample_texture);
        PS_ASSERT(m_tone_mapping_pipeline->bake(), "Could not bake ToneMapping pipeline")

        m_tone_mapping_pass = RenderPass::create({
            .debug_name = "ToneMappingPass",
            .target_framebuffer = m_tone_mapping_framebuffer,
        });
    }
}

std::vector<std::shared_ptr<Light>> DeferredRenderer::get_light_info() const {
    const auto light_entities = m_scene->get_entities_with<LightComponent>();

    std::vector<std::shared_ptr<Light>> lights;
    for (const auto& entity : light_entities) {
        const auto transform = entity.get_component<TransformComponent>();
        const auto light_component = entity.get_component<LightComponent>();

        if (light_component.type == Light::Type::Point) {
            auto light = std::make_shared<PointLight>(transform.position, light_component.color);
            lights.push_back(light);
        } else if (light_component.type == Light::Type::Directional) {
            auto direction = glm::vec3(0.0f, 0.0f, 1.0f); // Z+
            direction = glm::rotate(direction, transform.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
            direction = glm::rotate(direction, transform.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
            direction = glm::rotate(direction, transform.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
            direction = glm::normalize(direction);

            auto light = std::make_shared<DirectionalLight>(transform.position, direction, light_component.color);
            light->shadow_type = light_component.shadow_type;

            lights.push_back(light);
        }
    }

    return lights;
}

std::vector<Entity> DeferredRenderer::get_renderable_entities() const {
    std::vector<Entity> entities;
    for (const auto& entity : m_scene->get_entities_with<MeshRendererComponent>()) {
        const auto& mesh_renderer = entity.get_component<MeshRendererComponent>();

        if (mesh_renderer.mesh != nullptr && mesh_renderer.material != nullptr)
            entities.push_back(entity);
    }

    return entities;
}

} // namespace Phos
