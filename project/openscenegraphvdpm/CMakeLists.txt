include_directories(
    ${OPENSCENEGRAPH_INCLUDE_DIR}
    ${OSGVDPM_INCLUDE_DIR}
)

# osgvdpmanimationviewer
set(CMAKE_PROJECT_NAME osgvdpmanimationviewer)
add_executable(osgvdpmanimationviewer
    osgvdpmanimationviewer/AnimtkViewer.cpp
    osgvdpmanimationviewer/AnimtkViewerKeyHandler.cpp
    osgvdpmanimationviewer/AnimtkViewerGUI.cpp
)
include(${PROJECT_SOURCE_DIR}/config/link.cmake)

# osgvdpmcompositeviewer
set(CMAKE_PROJECT_NAME osgvdpmcompositeviewer)
add_executable(osgvdpmcompositeviewer osgvdpmcompositeviewer/osgvdpmcompositeviewer.cpp)
include(${PROJECT_SOURCE_DIR}/config/link.cmake)

# osgvdpmstaticviewer
set(CMAKE_PROJECT_NAME osgvdpmstaticviewer)
add_definitions(-DUSE_FREETYPE)
add_executable(osgvdpmstaticviewer osgvdpmstaticviewer/osgvdpmstaticviewer.cpp)
include(${PROJECT_SOURCE_DIR}/config/link.cmake)
