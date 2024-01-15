project(PhosEditor LANGUAGES CXX C)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
        main.cpp
        asset_watcher.cpp
        file_dialog.cpp
        editor_scene_manager.cpp

        asset_tools/editor_material_helper.cpp
        asset_tools/editor_cubemap_helper.cpp
        asset_tools/editor_prefab_helper.cpp
        asset_tools/asset_importer.cpp
        asset_tools/assimp_importer.cpp
        asset_tools/scene_serializer.cpp
        asset_tools/entity_serializer.cpp

        panels/viewport_panel.cpp
        panels/entity_hierarchy_panel.cpp
        panels/content_browser_panel.cpp
        panels/asset_inspector_panel.cpp
        panels/scene_configuration_panel.cpp

        panels/functional/entity_components_renderer.cpp

        imgui/imgui_impl.cpp
        imgui/vulkan/imgui_vulkan_impl.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/)

#
# Dependencies
#

# PhosEngine
target_link_libraries(${PROJECT_NAME} PRIVATE PhosEngine)

# yaml-cpp
target_link_libraries(${PROJECT_NAME} PUBLIC yaml-cpp)

# imgui
target_link_libraries(${PROJECT_NAME} PRIVATE imgui)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui_impl_glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui_impl_vulkan)

# assimp
target_link_libraries(${PROJECT_NAME} PRIVATE assimp)

# nfd
target_link_libraries(${PROJECT_NAME} PRIVATE nfd)
