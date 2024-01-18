cmake_minimum_required(VERSION 3.20)

project(Phos)

# Global Project settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add_compile_options(-Wall -Wpedantic -Wextra -Wshadow -Wconversion -Werror)
add_compile_options(-Wall -Wpedantic -Wextra -Wshadow -Wconversion)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

# PhosEngine
add_subdirectory(src/)

# Options
option(PHOS_BUILD_TESTS "Build PhosEngine Tests" ON)
option(PHOS_BUILD_EDITOR "Build Phos Editor" ON)
option(PHOS_BUILD_RUNTIME "Build Phos Runtime" ON)

if (PHOS_BUILD_TESTS)
    add_subdirectory(tests/)
endif ()

if (PHOS_BUILD_EDITOR)
    add_subdirectory(apps/editor/)
endif ()

if (PHOS_BUILD_RUNTIME)
    # add_subdirectory(apps/runtime/)
endif ()

#
# Dependencies
#

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/glfw)

# glm
add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/glm)

# stb
add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/stb/)

# spdlog
add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/spdlog)

# assimp
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/assimp)

# Spirv-reflect
set(SPIRV_REFLECT_EXECUTABLE OFF CACHE BOOL "" FORCE)
set(SPIRV_REFLECT_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SPIRV_REFLECT_STATIC_LIB ON CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/spirv-reflect)

# yaml-cpp
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "disable yaml tests")
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "disable yaml tools")
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "disable yaml contrib")

add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/yaml-cpp)

# Mono
function(mono_build_command cmd)
    execute_process(COMMAND bash -c ${cmd} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/vendor/mono/ RESULT_VARIABLE ret)
    if (ret EQUAL 1)
        message(FATAL_ERROR "Mono build failed")
    endif ()
endfunction()

if (NOT EXISTS ${CMAKE_SOURCE_DIR}/vendor/mono/build/)
    set(MONO_BUILD_PATH ${CMAKE_SOURCE_DIR}/vendor/mono/build)
    message("Building mono to: " ${MONO_BUILD_PATH})

    mono_build_command("autogen.sh --prefix=${MONO_BUILD_PATH}")
    mono_build_command("make -j 12")
    mono_build_command("make install")
endif ()

if (NOT EXISTS ${CMAKE_BINARY_DIR}/mono/lib/)
    message("Copying mono libraries to: " ${CMAKE_BINARY_DIR}/mono/lib/)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/vendor/mono/build/lib/ DESTINATION ${CMAKE_BINARY_DIR}/mono/lib/)
endif ()

# Tracy
option(TRACY_ENABLE "" ON)
option(TRACY_ON_DEMAND "" ON)
add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/tracy)

# imgui
add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/imgui)

# nfd
add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/nativefiledialog-extended)

# Catch 2
add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/Catch2)
