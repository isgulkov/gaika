cmake_minimum_required(VERSION 3.10)
project(dz04projections)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)

add_executable(dz04 src/dz04.cpp)

add_library(dz04_geometry lib/geometry.cpp)
target_link_libraries(dz04 dz04_geometry)

find_package(SDL2 REQUIRED)

target_link_libraries(dz04 ${SDL2_LIBRARIES})
target_include_directories(dz04 PRIVATE ${SDL2_INCLUDE_DIR})
