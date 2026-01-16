file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(imgui_shared SHARED ${SOURCES})
target_include_directories(imgui_shared INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
