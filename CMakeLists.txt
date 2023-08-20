cmake_minimum_required(VERSION 3.20)

project(Phos LANGUAGES CXX)

# Project settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create executable
add_executable(${PROJECT_NAME})

# target_compile_options(${PROJECT_NAME} BEFORE PRIVATE -Wall -Wpedantic -Wextra -Wshadow -Wconversion -Werror)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

# Sources and include directories
target_sources(${PROJECT_NAME} PRIVATE
        # Core
        src/core/application.cpp
        src/core/window.cpp
        src/core/entry_point.cpp
        src/core/uuid.cpp

        # Asset
        src/asset/asset_loader.cpp
        src/asset/asset_pack.cpp
        src/asset/model_asset.cpp

        # Scene
        src/scene/registry.cpp
        src/scene/scene.cpp
        src/scene/model_loader.cpp

        # Input
        src/input/input.cpp

        # Managers
        src/managers/texture_manager.cpp
        src/managers/shader_manager.cpp

        # Renderer
        src/renderer/mesh.cpp
        src/renderer/camera.cpp
        src/renderer/light.cpp

        src/renderer/deferred_renderer.cpp
        # src/renderer/forward_renderer.cpp

        # Renderer Backend
        src/renderer/backend/renderer.cpp
        src/renderer/backend/buffers.cpp
        src/renderer/backend/framebuffer.cpp
        src/renderer/backend/image.cpp
        src/renderer/backend/texture.cpp
        src/renderer/backend/command_buffer.cpp
        src/renderer/backend/render_pass.cpp
        src/renderer/backend/shader.cpp
        src/renderer/backend/graphics_pipeline.cpp
        src/renderer/backend/material.cpp
        src/renderer/backend/cubemap.cpp
        src/renderer/backend/presenter.cpp

        # Vulkan Backend
        src/renderer/backend/vulkan/vulkan_renderer.cpp
        src/renderer/backend/vulkan/vulkan_context.cpp
        src/renderer/backend/vulkan/vulkan_instance.cpp
        src/renderer/backend/vulkan/vulkan_physical_device.cpp
        src/renderer/backend/vulkan/vulkan_device.cpp
        src/renderer/backend/vulkan/vulkan_swapchain.cpp
        src/renderer/backend/vulkan/vulkan_shader.cpp
        src/renderer/backend/vulkan/vulkan_render_pass.cpp
        src/renderer/backend/vulkan/vulkan_graphics_pipeline.cpp
        src/renderer/backend/vulkan/vulkan_command_pool.cpp
        src/renderer/backend/vulkan/vulkan_command_buffer.cpp
        src/renderer/backend/vulkan/vulkan_queue.cpp
        src/renderer/backend/vulkan/vulkan_framebuffer.cpp
        src/renderer/backend/vulkan/vulkan_buffers.cpp
        src/renderer/backend/vulkan/vulkan_buffer.cpp
        src/renderer/backend/vulkan/vulkan_image.cpp
        src/renderer/backend/vulkan/vulkan_texture.cpp
        src/renderer/backend/vulkan/vulkan_descriptors.cpp
        src/renderer/backend/vulkan/vulkan_utils.cpp
        src/renderer/backend/vulkan/vulkan_renderer_api.cpp
        src/renderer/backend/vulkan/vulkan_material.cpp
        src/renderer/backend/vulkan/vulkan_cubemap.cpp
        src/renderer/backend/vulkan/vulkan_presenter.cpp

        # Editor
        editor/main.cpp
        editor/imgui/imgui_impl.cpp
        editor/imgui/vulkan/imgui_vulkan_impl.cpp

        # src/main.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC src/)

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

# glm
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)

# Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES})

# stb
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb/)
target_link_libraries(${PROJECT_NAME} PRIVATE stb)

# spdlog
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)

# assimp
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/assimp)
target_link_libraries(${PROJECT_NAME} PRIVATE assimp)

# Spirv-reflect
set(SPIRV_REFLECT_EXECUTABLE OFF CACHE BOOL "" FORCE)
set(SPIRV_REFLECT_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SPIRV_REFLECT_STATIC_LIB ON CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/spirv-reflect)
target_link_libraries(${PROJECT_NAME} PRIVATE spirv-reflect-static)

# yaml-cpp
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "disable yaml tests")
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "disable yaml tools")
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "disable yaml contrib")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/yaml-cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE yaml-cpp)

# imgui
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui_impl_glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui_impl_vulkan)


# Catch 2
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/Catch2)

#
# Tests
#
add_executable(tests)

target_sources(tests PRIVATE
        tests/scene/registry_tests.cpp
        src/scene/registry.cpp

        tests/scene/scene_tests.cpp
        src/scene/scene.cpp
        src/core/uuid.cpp
)

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
target_link_libraries(tests PRIVATE spdlog::spdlog)
target_link_libraries(tests PRIVATE glm::glm)

target_include_directories(tests PUBLIC src/)
