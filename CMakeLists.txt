cmake_minimum_required(VERSION 3.20)

project(vulkan-renderer LANGUAGES CXX)

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
        src/core/window.cpp
        src/core/glfw_window.cpp

        src/renderer/vulkan_instance.cpp
        src/renderer/vulkan_physical_device.cpp
        src/renderer/vulkan_device.cpp
        src/renderer/vulkan_swapchain.cpp
        src/renderer/vulkan_shader_module.cpp
        src/renderer/vulkan_render_pass.cpp
        src/renderer/vulkan_graphics_pipeline.cpp
        src/renderer/vulkan_command_pool.cpp
        src/renderer/vulkan_command_buffer.cpp
        src/renderer/vulkan_queue.cpp
        src/renderer/vulkan_framebuffer.cpp
        src/renderer/vulkan_buffers.cpp
        src/renderer/vulkan_buffer.cpp
        src/renderer/vulkan_image.cpp
        src/renderer/vulkan_texture.cpp
        src/renderer/vulkan_descriptors.cpp
        src/renderer/vulkan_utils.cpp

        src/renderer/vulkan_context.cpp

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
target_include_directories(${PROJECT_NAME} PRIVATE vendor/stb/)

# spdlog
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)

# assimp
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/assimp)
target_link_libraries(${PROJECT_NAME} PRIVATE assimp)

# Spirv-reflect
set(SPIRV_REFLECT_EXECUTABLE OFF CACHE BOOL "" FORCE)
set(SPIRV_REFLECT_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SPIRV_REFLECT_STATIC_LIB ON CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/spirv-reflect)
target_link_libraries(${PROJECT_NAME} PRIVATE spirv-reflect-static)