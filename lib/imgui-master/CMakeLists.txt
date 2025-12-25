file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(imgui STATIC ${SOURCES})
target_include_directories(imgui INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
