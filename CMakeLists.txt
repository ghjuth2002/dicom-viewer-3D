cmake_minimum_required(VERSION 3.5)

project(dicomViewer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_SOURCES
        main.cpp
        widget.cpp
        widget.h
        widget.ui
)

find_package(Qt5 COMPONENTS
    Widgets
    Core
    Gui
    Quick
    OpenGL
    REQUIRED)

# Find package VTK
find_package(VTK
    9.0
    REQUIRED
    COMPONENTS
        GUISupportQt
        IOLegacy)

# VTK Libraries
set(VTK_DIR $ENV{VTK_DIR})
find_package(VTK REQUIRED NO_MODULE)
include(${VTK_USE_FILE})

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    Resources.qrc
)

add_subdirectory(SceneView)
add_subdirectory(GUI)
add_subdirectory(Common)
add_subdirectory(EventHandler)

target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt5::Widgets
    Qt5::Core
    Qt5::OpenGL
    VTK::GUISupportQt
    VTK::IOLegacy
    ${VTK_LIBRARIES}
    SceneView
    EventsModel
    ConfigDialog
)

vtk_module_autoinit(
    TARGETS ${PROJECT_NAME}
    MODULES VTK::GUISupportQt VTK::IOLegacy)



