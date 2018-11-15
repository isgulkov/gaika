cmake_minimum_required(VERSION 3.10)
project(dz04projections)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)

add_library(dz04geometry src/geometry.cpp)

add_executable(dz04projections src/main.cpp)

target_link_libraries(dz04projections dz04geometry)

find_package(SDL2 REQUIRED)
target_link_libraries(dz04projections ${SDL2_LIBRARIES})
target_include_directories(dz04projections PRIVATE ${SDL2_INCLUDE_DIRS})
