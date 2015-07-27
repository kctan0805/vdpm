include_directories(
    ${GFX_INCLUDE_DIR}
    ${MIXKIT_INCLUDE_DIR}
    ${OPENSCENEGRAPH_INCLUDE_DIR}
    ${OSGVDPM_INCLUDE_DIR}
    ${VDPM_INCLUDE_DIR}
)

# osgvdpmconv
set(CMAKE_PROJECT_NAME osgvdpmconv)
add_executable(osgvdpmconv
    OrientationConverter.cpp
    OrientationConverter.h
    osgconv.cpp
    SRMeshConverter.cpp
    SRMeshConverter.h
)
include(${PROJECT_SOURCE_DIR}/config/link.cmake)
