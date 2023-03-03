cmake_minimum_required(VERSION 3.20)

project(VulkanRenderer)

# Create executable
add_executable(${PROJECT_NAME})

# Project settings
set(CMAKE_CXX_STANDARD 20)
target_compile_options(${PROJECT_NAME} BEFORE PRIVATE -Wall -Wpedantic -Wextra -Wshadow -Wconversion -Werror)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

# Sources and include directories
target_sources(${PROJECT_NAME} PRIVATE 
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