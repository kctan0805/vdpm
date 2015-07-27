
INCLUDE_DIRECTORIES(
    ${OPENSCENEGRAPH_INCLUDE_DIR}
    ${OSGEARTH_INCLUDE_DIR}
)

# applications
SET(TARGET_DEFAULT_PREFIX "application_")

# osgearth_viewer
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/applications/osgearth_viewer)
SET(TARGET_NAME "osgearth_viewer")
SET(CMAKE_PROJECT_NAME "${TARGET_DEFAULT_PREFIX}${TARGET_NAME}")

SET(TARGET_SRC ${SRC_DIR}/osgearth_viewer.cpp )
SET(TARGET_H ${SRC_DIR}/osgPlugins.h )

ADD_EXECUTABLE(${CMAKE_PROJECT_NAME} ${TARGET_SRC}  ${TARGET_H})
include(${PROJECT_SOURCE_DIR}/config/link.cmake)

# osgearth_overlayviewer
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/applications/osgearth_overlayviewer)
SET(TARGET_NAME "osgearth_overlayviewer")
SET(CMAKE_PROJECT_NAME "${TARGET_DEFAULT_PREFIX}${TARGET_NAME}")

SET(TARGET_SRC ${SRC_DIR}/osgearth_overlayviewer.cpp )
SET(TARGET_H ${SRC_DIR}/osgPlugins.h )

ADD_EXECUTABLE(${CMAKE_PROJECT_NAME} ${TARGET_SRC}  ${TARGET_H})
include(${PROJECT_SOURCE_DIR}/config/link.cmake)

# osgearth_version
set(SRC_DIR ${OSGEARTH_SOURCE_DIR}/src/applications/osgearth_version)
SET(TARGET_NAME "osgearth_version")
SET(CMAKE_PROJECT_NAME "${TARGET_DEFAULT_PREFIX}${TARGET_NAME}")

SET(TARGET_SRC ${SRC_DIR}/osgearth_version.cpp )

ADD_EXECUTABLE(${CMAKE_PROJECT_NAME} ${TARGET_SRC})
include(${PROJECT_SOURCE_DIR}/config/link.cmake)

# osgearth_city
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/applications/osgearth_city)
SET(TARGET_NAME "osgearth_city")
SET(CMAKE_PROJECT_NAME "${TARGET_DEFAULT_PREFIX}${TARGET_NAME}")

SET(TARGET_SRC ${SRC_DIR}/osgearth_city.cpp )
SET(TARGET_H ${SRC_DIR}/osgPlugins.h )

ADD_EXECUTABLE(${CMAKE_PROJECT_NAME} ${TARGET_SRC}  ${TARGET_H})
include(${PROJECT_SOURCE_DIR}/config/link.cmake)
