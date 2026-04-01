cmake_minimum_required(VERSION 3.31.4)
project(middle VERSION 1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

add_library(raylib STATIC IMPORTED)
set_target_properties(raylib PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/raylib-5.5_win64_msvc16/lib/raylib.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/lib/raylib-5.5_win64_msvc16/include"
)

# export all symbolds.. for imgui to work as a shared library
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)


# globbing
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/program/*.cpp")
file(GLOB HEADERS "${CMAKE_SOURCE_DIR}/include/*.h")
file(GLOB SYSTEMS_SOURCES "${CMAKE_SOURCE_DIR}/assets/systems/*.cpp")
file(GLOB COMPONENT_HEADERS "${CMAKE_SOURCE_DIR}/assets/components/*.h")
file(GLOB COMPONENT_SOURCES "${CMAKE_SOURCE_DIR}/assets/components/*.cpp")
file(GLOB ENTITIES_HEADERS "${CMAKE_SOURCE_DIR}/src/middle_entities/*.h")

# library game, hot reloadable I guess
add_library(game SHARED 
    ${SOURCES} 
    ${HEADERS} 
    ${SYSTEMS_SOURCES} 
    ${COMPONENT_HEADERS} 
    ${COMPONENT_SOURCES} 
    ${ENTITIES_HEADERS}
)
target_include_directories(game PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(game PUBLIC ${CMAKE_SOURCE_DIR}/assets/components)
target_include_directories(game PUBLIC ${CMAKE_SOURCE_DIR}/src/middle_components)
target_include_directories(game PUBLIC ${CMAKE_SOURCE_DIR}/src/middle_entities)
target_include_directories(game PUBLIC ${CMAKE_SOURCE_DIR}/lib/raylib-5.5_win64_msvc16/include)

# middle executable?, not hot reloadable, though not sure
add_executable(middle 
    src/main.cpp 
    src/win32_platform.cpp 
    src/middle_imgui.cpp 
    src/Renderer.cpp
    src/InputSystem.cpp
    src/FileDropSystem.cpp
)
target_include_directories(middle PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(middle PUBLIC ${CMAKE_SOURCE_DIR}/assets/components)
target_include_directories(middle PUBLIC ${CMAKE_SOURCE_DIR}/src/middle_components)

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

