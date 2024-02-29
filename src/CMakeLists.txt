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
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(glfw)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

# glm
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.1
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(glm)

target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE) # Only for Vulkan
target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_FORCE_RADIANS)
target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_ENABLE_EXPERIMENTAL)

target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)

# Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES})

# stb
add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/stb ${CMAKE_CURRENT_BINARY_DIR}/stb)
target_link_libraries(${PROJECT_NAME} PRIVATE stb)

# spdlog
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.13.0
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(spdlog)

target_link_libraries(${PROJECT_NAME} PUBLIC spdlog)

# assimp
FetchContent_Declare(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp
        GIT_TAG v5.3.1
        GIT_PROGRESS TRUE
)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(assimp)
target_link_libraries(${PROJECT_NAME} PUBLIC assimp::assimp)

# Spirv-reflect
FetchContent_Declare(
        spirv-reflect
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Reflect.git
        GIT_TAG vulkan-sdk-1.3.275.0
        GIT_PROGRESS TRUE
)
set(SPIRV_REFLECT_EXECUTABLE OFF CACHE BOOL "" FORCE)
set(SPIRV_REFLECT_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SPIRV_REFLECT_STATIC_LIB ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(spirv-reflect)
target_link_libraries(${PROJECT_NAME} PRIVATE spirv-reflect-static)

# yaml-cpp
FetchContent_Declare(
        yaml-cpp
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG 0.8.0
        GIT_PROGRESS TRUE
)
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "disable yaml tests")
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "disable yaml tools")
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "disable yaml contrib")

FetchContent_MakeAvailable(yaml-cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC yaml-cpp)

# Mono
target_link_libraries(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/vendor/mono/libmonosgen-2.0.a)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/vendor/mono/include/)

# Tracy
FetchContent_Declare(
        tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy.git
        GIT_TAG v0.10
        GIT_PROGRESS TRUE
)

option(TRACY_ENABLE "" ON)
option(TRACY_ON_DEMAND "" ON)

FetchContent_MakeAvailable(tracy)
target_link_libraries(${PROJECT_NAME} PUBLIC Tracy::TracyClient)
