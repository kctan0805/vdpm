include_directories(
    ${OPENSCENEGRAPH_INCLUDE_DIR}
    ${OSGVDPM_INCLUDE_DIR}
)

# osgvdpmcompositeviewer
set(CMAKE_PROJECT_NAME osgvdpmcompositeviewer)
add_executable(osgvdpmcompositeviewer osgvdpmcompositeviewer/osgvdpmcompositeviewer.cpp)
include(${PROJECT_SOURCE_DIR}/config/link.cmake)

# osgvdpmstaticviewer
set(CMAKE_PROJECT_NAME osgvdpmstaticviewer)
add_definitions(-DUSE_FREETYPE)
add_executable(osgvdpmstaticviewer osgvdpmstaticviewer/osgvdpmstaticviewer.cpp)
include(${PROJECT_SOURCE_DIR}/config/link.cmake)
