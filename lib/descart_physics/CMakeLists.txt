cmake_minimum_required(VERSION 3.31.4)
project(DESCART_PHYSICS VERSION 1 LANGUAGES CXX)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)


file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
add_library(descart_physics ${SOURCES} ${HEADERS})

target_include_directories(descart_physics PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
