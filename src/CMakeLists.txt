project(PhosEngine LANGUAGES C CXX)

add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
        # Core
        core/application.cpp
        core/window.cpp
        core/entry_point.cpp
        core/uuid.cpp
        core/project.cpp

        # Asset
        asset/asset_loader.cpp
        asset/asset_pack.cpp
        asset/model_asset.cpp
        asset/runtime_asset_manager.cpp
        asset/editor_asset_manager.cpp
        asset/prefab_loader.cpp

        # Scene
        scene/registry.cpp
        scene/scene.cpp
        scene/model_loader.cpp
        scene/entity_deserializer.cpp

        # Scripting
        scripting/scripting_engine.cpp
        scripting/class_handle.cpp
        scripting/class_instance_handle.cpp
        scripting/script_glue.cpp
        scripting/scripting_system.cpp
        scripting/class_field.cpp

        # Input
        input/input.cpp

        # Managers
        managers/texture_manager.cpp
        managers/shader_manager.cpp

        # Renderer
        renderer/mesh.cpp
        renderer/camera.cpp
        renderer/light.cpp

        renderer/deferred_renderer.cpp
        # src/renderer/forward_renderer.cpp

        # Renderer Backend
        renderer/backend/renderer.cpp
        renderer/backend/buffers.cpp
        renderer/backend/framebuffer.cpp
        renderer/backend/image.cpp
        renderer/backend/texture.cpp
        renderer/backend/command_buffer.cpp
        renderer/backend/render_pass.cpp
        renderer/backend/shader.cpp
        renderer/backend/graphics_pipeline.cpp
        renderer/backend/material.cpp
        renderer/backend/cubemap.cpp
        renderer/backend/presenter.cpp
        renderer/backend/compute_pipeline.cpp

        # Vulkan Backend
        renderer/backend/vulkan/vulkan_renderer.cpp
        renderer/backend/vulkan/vulkan_context.cpp
        renderer/backend/vulkan/vulkan_instance.cpp
        renderer/backend/vulkan/vulkan_physical_device.cpp
        renderer/backend/vulkan/vulkan_device.cpp
        renderer/backend/vulkan/vulkan_swapchain.cpp
        renderer/backend/vulkan/vulkan_shader.cpp
        renderer/backend/vulkan/vulkan_render_pass.cpp
        renderer/backend/vulkan/vulkan_graphics_pipeline.cpp
        renderer/backend/vulkan/vulkan_command_pool.cpp
        renderer/backend/vulkan/vulkan_command_buffer.cpp
        renderer/backend/vulkan/vulkan_queue.cpp
        renderer/backend/vulkan/vulkan_framebuffer.cpp
        renderer/backend/vulkan/vulkan_buffers.cpp
        renderer/backend/vulkan/vulkan_buffer.cpp
        renderer/backend/vulkan/vulkan_image.cpp
        renderer/backend/vulkan/vulkan_texture.cpp
        renderer/backend/vulkan/vulkan_descriptors.cpp
        renderer/backend/vulkan/vulkan_utils.cpp
        renderer/backend/vulkan/vulkan_renderer_api.cpp
        renderer/backend/vulkan/vulkan_material.cpp
        renderer/backend/vulkan/vulkan_cubemap.cpp
        renderer/backend/vulkan/vulkan_presenter.cpp
        renderer/backend/vulkan/vulkan_compute_pipeline.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)

#
# TODO: Shaders
#

#
# Dependencies
#

# GLFW
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

# glm
target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE) # Only for Vulkan
target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_FORCE_RADIANS)

target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)

# Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES})

# stb
target_link_libraries(${PROJECT_NAME} PRIVATE stb)

# spdlog
target_link_libraries(${PROJECT_NAME} PUBLIC spdlog::spdlog)

# assimp
target_link_libraries(${PROJECT_NAME} PRIVATE assimp)

# Spirv-reflect
target_link_libraries(${PROJECT_NAME} PRIVATE spirv-reflect-static)

# yaml-cpp
target_link_libraries(${PROJECT_NAME} PRIVATE yaml-cpp)

# Mono
target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/vendor/mono/build/lib/libmono-2.0.so)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/vendor/mono/build/include/mono-2.0/)

# Tracy
target_link_libraries(${PROJECT_NAME} PUBLIC Tracy::TracyClient)
