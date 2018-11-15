cmake_minimum_required(VERSION 3.10)
project(dz06lighting)

set(CMAKE_CXX_STANDARD 11)

add_executable(dz04projections main.cpp)

find_package(SDL2 REQUIRED)

target_link_libraries(dz04projections ${SDL2_LIBRARIES})
target_include_directories(dz04projections PRIVATE ${SDL2_INCLUDE_DIRS})
