set(LIB_NAME EventsModel)

set(DEPENDENCIES
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    EventHandler
    )

file(GLOB_RECURSE SOURCES "include/*" "src/*")
add_library(${LIB_NAME} STATIC ${SOURCES})
target_include_directories(${LIB_NAME} PUBLIC "include")
target_link_libraries(${LIB_NAME} PUBLIC "${DEPENDENCIES}")



