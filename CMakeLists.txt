cmake_minimum_required(VERSION 3.31.4)
project(middle VERSION 1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

#middle paths
set(MIDDLE_COMPONENTS_DIR "${CMAKE_SOURCE_DIR}/game/components")
set(COMPONENT_TEMPLATE_HEADER "${CMAKE_SOURCE_DIR}/middle/editorData/component_template.h")
set(COMPONENT_TEMPLATE_SOURCE "${CMAKE_SOURCE_DIR}/middle/editorData/component_template.cpp")
set(SYSTEM_TEMPLATE_SOURCE "${CMAKE_SOURCE_DIR}/middle/editorData/system_template.cpp")
set(SYSTEMS_DIR "${CMAKE_SOURCE_DIR}/game/systems")
set(MIDDLE_SCENE_FILE_EXTENSION ".midsc")
set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")
set(SCENES_DIR "${ASSETS_DIR}/scenes")
set(TEXTURES_DIR "${ASSETS_DIR}/textures")
set(SHADERS_DIR "${ASSETS_DIR}/shaders")
set(TEMP_DIR "${CMAKE_SOURCE_DIR}/middle/temp")
set(EDITOR_STATE_FILE "${CMAKE_SOURCE_DIR}/middle/editorData/editor_state${MIDDLE_SCENE_FILE_EXTENSION}")

#bubble paths
set(EQUATION_DIR "${ASSETS_DIR}/equations")
set(BUBBLE_TREE_DIR "${ASSETS_DIR}/bubbleTree")
set(WORD_PROBLEMS_DIR "${ASSETS_DIR}/wordProblems")


add_library(raylib STATIC IMPORTED)
set_target_properties(raylib PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/raylib-5.5_win64_msvc16/lib/raylib.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/lib/raylib-5.5_win64_msvc16/include"
)

# export all symbolds.. for imgui to work as a shared library
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)


# globbing
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/game/src/*.cpp")
file(GLOB HEADERS "${CMAKE_SOURCE_DIR}/include/*.h")
file(GLOB SYSTEM_SOURCES "${SYSTEMS_DIR}/*.cpp")
file(GLOB COMPONENT_HEADERS "${MIDDLE_COMPONENTS_DIR}/*.h")
file(GLOB COMPONENT_SOURCES "${MIDDLE_COMPONENTS_DIR}/*.cpp")
file(GLOB ENTITIES_HEADERS "${CMAKE_SOURCE_DIR}/middle/middleEntities/*.h")

# library game, hot reloadable I guess
add_library(game SHARED 
    ${SOURCES} 
    ${HEADERS} 
    ${SYSTEM_SOURCES} 
    ${COMPONENT_HEADERS} 
    ${COMPONENT_SOURCES} 
    ${ENTITIES_HEADERS}
)
target_include_directories(game PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(game PUBLIC ${MIDDLE_COMPONENTS_DIR})
target_include_directories(game PUBLIC ${CMAKE_SOURCE_DIR}/middle/middleEntities)
target_include_directories(game PUBLIC ${CMAKE_SOURCE_DIR}/lib/raylib-5.5_win64_msvc16/include)

# middle executable?, not hot reloadable, though not sure
add_executable(middle 
    middle/src/main.cpp 
    middle/src/win32_platform.cpp 
    middle/src/middle_imgui.cpp 
    middle/middleSystems/Renderer.cpp
    middle/middleSystems/InputSystem.cpp
    middle/middleSystems/FileDropSystem.cpp
    middle/src/assets_loading.cpp
    ## todo
    game/src/game_state.cpp
)
target_include_directories(middle PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(middle PUBLIC ${MIDDLE_COMPONENTS_DIR})

add_subdirectory(${CMAKE_SOURCE_DIR}/lib/descart_physics)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/rlImGui-main)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/imgui-master)

target_include_directories(rlImGui PUBLIC ${CMAKE_SOURCE_DIR}/lib/raylib-5.5_win64_msvc16/include)

target_link_libraries(game PUBLIC descart_physics imgui_shared)
target_link_libraries(middle PUBLIC descart_physics raylib rlImGui)
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

configure_file(config.h.in "${CMAKE_SOURCE_DIR}/config.h")
target_include_directories(middle PRIVATE "${CMAKE_SOURCE_DIR}")
target_include_directories(game PRIVATE "${CMAKE_SOURCE_DIR}")
