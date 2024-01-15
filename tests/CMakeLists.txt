project(PhosEngineTests LANGUAGES CXX C)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
        # scene
        scene/registry_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/scene/registry.cpp

        scene/scene_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/scene/scene.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE Catch2::Catch2WithMain)
target_link_libraries(${PROJECT_NAME} PRIVATE PhosEngine)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
