cmake_minimum_required(VERSION 3.31.4)
project(middle VERSION 1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

set(MIDDLE_SCENE_FILE_EXTENSION ".midsc")

#middle game paths
set(MIDDLE_GAME_DIR "${CMAKE_SOURCE_DIR}/MiddleGame")
set(MIDDLE_COMPONENTS_DIR "${MIDDLE_GAME_DIR}/game/components")
set(SYSTEMS_DIR "${MIDDLE_GAME_DIR}/game/systems")
set(ASSETS_DIR "${MIDDLE_GAME_DIR}/assets")
set(SCENES_DIR "${ASSETS_DIR}/scenes")
set(TEXTURES_DIR "${ASSETS_DIR}/textures")
set(SHADERS_DIR "${ASSETS_DIR}/shaders")
set(SOUNDS_DIR "${ASSETS_DIR}/sounds")
set(FONTS_DIR "${ASSETS_DIR}/fonts")
set(TEMP_DIR "${CMAKE_SOURCE_DIR}/MiddleGame/game/temp")

#bubble paths
set(EQUATION_DIR "${ASSETS_DIR}/equations")
set(BUBBLE_TREE_DIR "${ASSETS_DIR}/bubbleTree")
set(WORD_PROBLEMS_DIR "${ASSETS_DIR}/wordProblems")

#middle raylib paths
set(MIDDLE_RAYLIB_DIR "${CMAKE_SOURCE_DIR}/MiddleRaylib")
set(COMPONENT_TEMPLATE_HEADER "${MIDDLE_RAYLIB_DIR}/middle/editorData/component_template.h")
set(SYSTEM_TEMPLATE_SOURCE "${MIDDLE_RAYLIB_DIR}/middle/editorData/system_template.cpp")
set(COMPONENT_TEMPLATE_SOURCE "${MIDDLE_RAYLIB_DIR}/middle/editorData/component_template.cpp")
set(EDITOR_STATE_FILE "${MIDDLE_RAYLIB_DIR}/middle/editorData/editor_state${MIDDLE_SCENE_FILE_EXTENSION}")


add_library(raylib STATIC IMPORTED)
set_target_properties(raylib PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/raylib-5.5_win64_msvc16/lib/raylib.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/lib/raylib-5.5_win64_msvc16/include"
)

# export all symbolds.. for imgui to work as a shared library
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)


# globbing
file(GLOB SOURCES "${MIDDLE_GAME_DIR}/game/src/*.cpp")
file(GLOB HEADERS "${MIDDLE_GAME_DIR}/include/*.h")
file(GLOB SYSTEM_SOURCES "${SYSTEMS_DIR}/*.cpp")
file(GLOB COMPONENT_HEADERS "${MIDDLE_COMPONENTS_DIR}/*.h")
file(GLOB COMPONENT_SOURCES "${MIDDLE_COMPONENTS_DIR}/MidComp/*.cpp")
file(GLOB ENTITIES_HEADERS "${MIDDLE_RAYLIB_DIR}/middle/middleEntities/*.h")

# library game, (it's not only game, TODO rename)
add_library(game SHARED 
    ${SOURCES} 
    ${HEADERS} 
    ${SYSTEM_SOURCES} 
    ${COMPONENT_HEADERS} 
    ${COMPONENT_SOURCES} 
    ${ENTITIES_HEADERS}
)
target_include_directories(game PUBLIC ${MIDDLE_GAME_DIR}/include)
target_include_directories(game PUBLIC ${MIDDLE_COMPONENTS_DIR})
target_include_directories(game PUBLIC ${MIDDLE_RAYLIB_DIR}/middle/middleEntities)

# raylib middle implementation
add_executable(middle 
    ${MIDDLE_RAYLIB_DIR}/middle/src/main.cpp 
    ${MIDDLE_RAYLIB_DIR}/middle/src/win32_platform.cpp 
    ${MIDDLE_RAYLIB_DIR}/middle/src/middle_imgui.cpp 
    ${MIDDLE_RAYLIB_DIR}/middle/middleSystems/Renderer.cpp
    ${MIDDLE_RAYLIB_DIR}/middle/middleSystems/InputSystem.cpp
    ${MIDDLE_RAYLIB_DIR}/middle/middleSystems/FileDropSystem.cpp
    ${MIDDLE_RAYLIB_DIR}/middle/src/assets_loading.cpp
    ## todo?
    ${MIDDLE_GAME_DIR}/include/middle_state.h
    ${MIDDLE_GAME_DIR}/game/src/game_state.cpp
)
target_include_directories(middle PUBLIC ${MIDDLE_GAME_DIR}/include)
target_include_directories(middle PUBLIC ${MIDDLE_COMPONENTS_DIR})
target_include_directories(middle PUBLIC ${MIDDLE_RAYLIB_DIR}/middle/middleSystems)

add_subdirectory(${CMAKE_SOURCE_DIR}/lib/rlImGui-main)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/imgui-master)

target_include_directories(rlImGui PUBLIC ${CMAKE_SOURCE_DIR}/lib/raylib-5.5_win64_msvc16/include)

target_link_libraries(game PUBLIC imgui_shared)
target_link_libraries(middle PUBLIC raylib rlImGui)
target_link_libraries(rlImGui PUBLIC raylib winmm)


set_property(TARGET middle PROPERTY WIN32_EXECUTABLE TRUE)
if (MSVC)
    target_link_options(middle PRIVATE /ENTRY:mainCRTStartup)
endif()


# Copy DLLs to executable output directory
add_custom_command(TARGET middle POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:imgui_shared>
    $<TARGET_FILE_DIR:middle>
)

configure_file("${MIDDLE_GAME_DIR}/config.h.in" "${MIDDLE_GAME_DIR}/config.h")
target_include_directories(middle PRIVATE "${MIDDLE_GAME_DIR}")
target_include_directories(game PRIVATE "${MIDDLE_GAME_DIR}")
