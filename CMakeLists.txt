cmake_minimum_required(VERSION 3.10)
project(dz_gaika)

set(CMAKE_CXX_STANDARD 11)

add_library(dz04_matrices lib/matrices.cpp)
target_include_directories(dz04_matrices PUBLIC include)

add_library(dz04_geometry lib/geometry.cpp)
target_include_directories(dz04_geometry PUBLIC include)
# TODO: <- I do need this when hpp/cpp are separated, right?
#target_link_libraries(dz04_geometry dz04_matrices)

add_library(dz04_display2d lib/display2d.cpp)
target_include_directories(dz04_display2d PUBLIC include)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#sudo ln -s /usr/local/Cellar/qt/5.11.2/mkspecs/ /usr/local/mkspecs
#sudo ln -s /usr/local/Cellar/qt/5.11.2/plugins/ /usr/local/plugins
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
target_link_libraries(dz04_display2d Qt5::Core Qt5::Widgets)

#set_target_properties(dz04_display2d PROPERTIES AUTOMOC TRUE)
#set_target_properties(dz04_display2d PROPERTIES AUTOUIC TRUE)

add_executable(dz04_qt src/dz04_qt.cpp resources/resources.qrc)
target_link_libraries(dz04_qt dz04_display2d dz04_matrices dz04_geometry)

set_target_properties(dz04_qt PROPERTIES AUTOMOC TRUE)
