cmake_minimum_required(VERSION 3.10)
project(dz04projections)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)

add_executable(dz04projections src/main.cpp)

add_library(dz04geometry src/geometry.cpp)
target_link_libraries(dz04projections dz04geometry)

find_package(SDL2 REQUIRED)

add_subdirectory(vendor/SDL2_gfx)

target_link_libraries(dz04projections ${SDL2_LIBRARIES} SDL2_gfx)
target_include_directories(dz04projections PRIVATE ${SDL2_INCLUDE_DIR})

#list(APPEND CMAKE_PREFIX_PATH "/usr/local/Cellar/sdl2_gfx/1.0.4/")
