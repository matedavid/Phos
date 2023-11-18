cmake_minimum_required(VERSION 3.20)

project(Phos LANGUAGES CXX)

# Project settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create executable
add_library(${PROJECT_NAME})

# target_compile_options(${PROJECT_NAME} BEFORE PRIVATE -Wall -Wpedantic -Wextra -Wshadow -Wconversion -Werror)
target_compile_options(${PROJECT_NAME} BEFORE PRIVATE -Wall -Wpedantic -Wextra -Wshadow -Wconversion)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

# Sources and include directories
target_sources(${PROJECT_NAME} PRIVATE
        # Core
        src/core/application.cpp
        src/core/window.cpp
        src/core/entry_point.cpp
        src/core/uuid.cpp
        src/core/project.cpp

        # Asset
        src/asset/asset_loader.cpp
        src/asset/asset_pack.cpp
        src/asset/model_asset.cpp
        src/asset/runtime_asset_manager.cpp
        src/asset/editor_asset_manager.cpp
        src/asset/prefab_loader.cpp

        # Scene
        src/scene/registry.cpp
        src/scene/scene.cpp
        src/scene/model_loader.cpp
        src/scene/scene_serializer.cpp
        src/scene/entity_deserializer.cpp

        # Scripting
        src/scripting/scripting_engine.cpp
        src/scripting/class_handle.cpp
        src/scripting/class_instance_handle.cpp
        src/scripting/script_glue.cpp
        src/scripting/scripting_system.cpp
        src/scripting/class_field.cpp

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
        src/renderer/backend/compute_pipeline.cpp

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
        src/renderer/backend/vulkan/vulkan_compute_pipeline.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC src/)

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw)
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)

# glm
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm)
target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)

# Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES})

# stb
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb/)
target_link_libraries(${PROJECT_NAME} PRIVATE stb)

# spdlog
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog)
target_link_libraries(${PROJECT_NAME} PUBLIC spdlog::spdlog)

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
target_link_libraries(${PROJECT_NAME} PUBLIC yaml-cpp)

# imgui
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui)

# Catch 2
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/Catch2)

# Mono
if (NOT EXISTS ${CMAKE_BINARY_DIR}/mono/lib/)
    message("Copying mono libraries to: " ${CMAKE_BINARY_DIR}/mono/lib/)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/vendor/mono/build/lib/ DESTINATION ${CMAKE_BINARY_DIR}/mono/lib/)
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vendor/mono/build/lib/libmono-2.0.so)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vendor/mono/build/include/mono-2.0/)

# nativefiledialog
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/nativefiledialog)

#
# Editor
#
add_executable(PhosEditor
        editor/main.cpp
        editor/asset_watcher.cpp
        editor/file_dialog.cpp
        editor/editor_scene_manager.cpp

        editor/asset_tools/editor_material_helper.cpp
        editor/asset_tools/editor_cubemap_helper.cpp
        editor/asset_tools/asset_importer.cpp
        editor/asset_tools/assimp_importer.cpp

        editor/panels/viewport_panel.cpp
        editor/panels/entity_hierarchy_panel.cpp
        editor/panels/components_panel.cpp
        editor/panels/content_browser_panel.cpp
        editor/panels/asset_inspector_panel.cpp
        editor/panels/scene_configuration_panel.cpp

        editor/imgui/imgui_impl.cpp
        editor/imgui/vulkan/imgui_vulkan_impl.cpp
)

target_include_directories(PhosEditor PUBLIC editor/)

target_link_libraries(PhosEditor PUBLIC ${PROJECT_NAME})

target_link_libraries(PhosEditor PRIVATE imgui)
target_link_libraries(PhosEditor PRIVATE imgui_impl_glfw)
target_link_libraries(PhosEditor PRIVATE imgui_impl_vulkan)
target_link_libraries(PhosEditor PRIVATE nativefiledialog)
target_link_libraries(PhosEditor PRIVATE assimp)

#
# Application
#
add_executable(PhosApplication
        src/main.cpp
)

target_link_libraries(PhosApplication PUBLIC ${PROJECT_NAME})

#
# Tests
#
add_executable(tests)

target_sources(tests PRIVATE
        tests/scene/registry_tests.cpp
        tests/scene/scene_tests.cpp

        src/scene/registry.cpp
        src/scene/scene.cpp
        src/core/uuid.cpp
)

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
target_link_libraries(tests PRIVATE spdlog::spdlog)
target_link_libraries(tests PRIVATE glm::glm)
target_link_libraries(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vendor/mono/build/lib/libmono-2.0.so)
target_include_directories(tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vendor/mono/build/include/mono-2.0/)

target_include_directories(tests PUBLIC src/)
