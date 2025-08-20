set(LIB_NAME SceneView)

set(DEPENDENCIES
    Qt5::Core
    Qt5::Widgets
    Qt5::OpenGL
    Qt5::Gui
    VTK::GUISupportQt
    VTK::IOLegacy
    ${VTK_LIBRARIES}
    EventHandler
    )

file(GLOB_RECURSE SOURCES "include/*" "src/*")
add_library(${LIB_NAME} STATIC ${SOURCES})
target_include_directories(${LIB_NAME} PUBLIC "include")
target_link_libraries(${LIB_NAME} PUBLIC "${DEPENDENCIES}")

vtk_module_autoinit(
    TARGETS ${PROJECT_NAME}
    MODULES VTK::GUISupportQt VTK::IOLegacy)

