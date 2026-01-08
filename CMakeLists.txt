cmake_minimum_required(VERSION 3.31.4)
project(middle VERSION 1 LANGUAGES CXX)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)

add_library(raylib STATIC IMPORTED)
set_target_properties(raylib PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/raylib-5.5_win64_msvc16/lib/raylib.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/lib/raylib-5.5_win64_msvc16/include"
)



# globbing
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/program/*.cpp")
file(GLOB SCRIPTS "${CMAKE_SOURCE_DIR}/assets/scripts/*.cpp")
file(GLOB HEADERS "${CMAKE_SOURCE_DIR}/include/*.h")
file(GLOB COMPONENTS "${CMAKE_SOURCE_DIR}/assets/components/*.h")

# library game, hot reloadable I guess
add_library(game SHARED ${SOURCES} ${HEADERS} ${SCRIPTS} ${COMPONENTS})
target_include_directories(game PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(game PUBLIC ${CMAKE_SOURCE_DIR}/assets/components)
target_include_directories(game PUBLIC ${CMAKE_SOURCE_DIR}/lib/raylib-5.5_win64_msvc16/include)

# middle executable?, not hot reloadable, though not sure
add_executable(middle 
    src/main.cpp 
    src/win32_platform.cpp 
    src/renderer.cpp
    src/middle_imgui.cpp
    src/program/middle_shape_utils.cpp
)
target_include_directories(middle PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(middle PUBLIC raylib winmm)

add_subdirectory(${CMAKE_SOURCE_DIR}/lib/descart_physics)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/rlImGui-main)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/imgui-master)

target_include_directories(rlImGui PUBLIC ${CMAKE_SOURCE_DIR}/lib/raylib-5.5_win64_msvc16/include)

target_link_libraries(game PUBLIC descart_physics)
target_link_libraries(middle PUBLIC rlImGui descart_physics)
target_link_libraries(rlImGui PUBLIC raylib winmm)
