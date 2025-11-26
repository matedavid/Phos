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

# imgui
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.91.9b-docking
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(imgui)

set(ROOT ${imgui_SOURCE_DIR})

add_library(imgui STATIC
        ${ROOT}/imgui.cpp
        ${ROOT}/imgui.h
        ${ROOT}/imstb_rectpack.h
        ${ROOT}/imstb_textedit.h
        ${ROOT}/imstb_truetype.h
        ${ROOT}/imgui_demo.cpp
        ${ROOT}/imgui_draw.cpp
        ${ROOT}/imgui_internal.h
        ${ROOT}/imgui_tables.cpp
        ${ROOT}/imgui_widgets.cpp
        ${ROOT}/misc/cpp/imgui_stdlib.cpp
)
target_include_directories(imgui PUBLIC
        $<BUILD_INTERFACE:${ROOT}>
        $<INSTALL_INTERFACE:include>)

function(create_backend BACKEND)
    set(NAME imgui_impl_${BACKEND})
    set(HEADER ${ROOT}/backends/${NAME}.h)

    add_library(${NAME} STATIC ${ROOT}/backends/${NAME}.cpp ${HEADER})

    target_link_libraries(${NAME} PUBLIC imgui)
    target_include_directories(${NAME} PUBLIC
            $<BUILD_INTERFACE:${ROOT}/backends>
            $<INSTALL_INTERFACE:include>)

    LIST(APPEND INSTALL_TARGETS ${NAME})
    LIST(APPEND INSTALL_HEADERS ${HEADER})
endfunction()

create_backend(glfw)

target_link_libraries(imgui_impl_glfw PRIVATE glfw)
    
create_backend(vulkan)
    
find_package(Vulkan REQUIRED)
target_link_libraries(imgui_impl_vulkan PRIVATE ${Vulkan_LIBRARIES})
target_include_directories(imgui_impl_vulkan PUBLIC ${Vulkan_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE imgui)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui_impl_glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui_impl_vulkan)

# nfd
FetchContent_Declare(
        nfd
        GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended.git
        GIT_TAG master
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(nfd)

target_link_libraries(${PROJECT_NAME} PRIVATE nfd)
