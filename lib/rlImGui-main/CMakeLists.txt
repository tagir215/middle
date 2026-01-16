file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(rlImGui STATIC ${SOURCES})
target_include_directories(rlImGui INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(rlImGui PUBLIC imgui_shared)
