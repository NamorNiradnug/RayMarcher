cmake_minimum_required(VERSION 3.21)

set(PROJECT_NAME RayMarcher)

project(${PROJECT_NAME} LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
# using Qt5 by default because kvantum doesn't support Qt6

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/shaderrenderer.cpp
    src/shaderrenderer.h
    src/settingsmenu.cpp
    src/settingsmenu.h
    resources.qrc)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt::Widgets)
