cmake_minimum_required(VERSION 3.10)
project(dz_gaika)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)

add_executable(dz04 src/dz04.cpp)

add_library(dz04_matrices lib/matrices.cpp)
target_link_libraries(dz04 dz04_matrices)

add_library(dz04_geometry lib/geometry.cpp)
# TODO: <- I do need this when hpp/cpp are separated, right?
#target_link_libraries(dz04_geometry dz04_matrices)
target_link_libraries(dz04 dz04_geometry)

add_library(dz04_obj_io lib/obj_io.cpp)
target_link_libraries(dz04 dz04_obj_io)

find_package(SDL2 REQUIRED)
target_link_libraries(dz04 ${SDL2_LIBRARIES})
target_include_directories(dz04 PRIVATE ${SDL2_INCLUDE_DIR})

add_library(dz04_display2d lib/display2d.cpp)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

#sudo ln -s /usr/local/Cellar/qt/5.11.2/mkspecs/ /usr/local/mkspecs
#sudo ln -s /usr/local/Cellar/qt/5.11.2/plugins/ /usr/local/plugins
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
target_link_libraries(dz04_display2d Qt5::Core Qt5::Widgets)

set_target_properties(dz04_display2d PROPERTIES AUTOMOC TRUE)

add_executable(dz04_qt src/dz04_qt.cpp)
target_link_libraries(dz04_qt dz04_display2d)

set_target_properties(dz04_qt PROPERTIES AUTOMOC TRUE)
