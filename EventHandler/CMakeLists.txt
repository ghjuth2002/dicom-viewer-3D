set(LIB_NAME EventHandler)

set(DEPENDENCIES
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Common
    )

file(GLOB_RECURSE SOURCES "include/*" "src/*")
add_library(${LIB_NAME} STATIC ${SOURCES})
target_include_directories(${LIB_NAME} PUBLIC "include")
target_link_libraries(${LIB_NAME} PUBLIC "${DEPENDENCIES}")



