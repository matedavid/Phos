cmake_minimum_required(VERSION 3.20)

project(Phos)

# Global Project settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add_compile_options(-Wall -Wpedantic -Wextra -Wshadow -Wconversion -Werror)
# add_compile_options(-Wall -Wpedantic -Wextra -Wshadow -Wconversion)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

# Enable FetchContent
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

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
