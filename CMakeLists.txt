cmake_minimum_required(VERSION 3.20)

project(Phos LANGUAGES CXX)

# Project settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create executable
add_executable(${PROJECT_NAME})

target_compile_options(${PROJECT_NAME} BEFORE PRIVATE -Wall -Wpedantic -Wextra -Wshadow -Wconversion -Werror)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

# Sources and include directories
target_sources(${PROJECT_NAME} PRIVATE
        src/core/application.cpp
        src/core/window.cpp
        src/core/entry_point.cpp

        src/input/input.cpp

        # Renderer
        src/renderer/static_mesh.cpp
        src/renderer/sub_mesh.cpp
        src/renderer/camera.cpp

        src/renderer/deferred_renderer.cpp

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

        src/main.cpp
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
