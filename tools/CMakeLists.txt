cmake_minimum_required(VERSION 3.20)

# Project settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#
# Libraries
#

# Assimp
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../vendor/assimp assimp/)

# yaml-cpp
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "disable yaml tests")
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "disable yaml tools")
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "disable yaml contrib")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../vendor/yaml-cpp yamlcpp/)

#
# Projects
#

# assimp importer
add_executable(AssimpImporter assimp_importer.cpp)
target_link_libraries(AssimpImporter PRIVATE assimp)
target_link_libraries(AssimpImporter PRIVATE yaml-cpp)
