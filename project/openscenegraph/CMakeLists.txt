set(OpenSceneGraph_SOURCE_DIR "${OPENSCENEGRAPH_SOURCE_DIR}")

INCLUDE_DIRECTORIES(
    ${OPENSCENEGRAPH_INCLUDE_DIR}
)

# applications
SET(TARGET_DEFAULT_PREFIX "application_")

# osgversion
set(SRC_DIR ${OpenSceneGraph_SOURCE_DIR}/applications/osgversion)
SET(TARGET_NAME "osgversion")
SET(CMAKE_PROJECT_NAME "${TARGET_DEFAULT_PREFIX}${TARGET_NAME}")

SET(TARGET_SRC ${SRC_DIR}/osgversion.cpp )

ADD_EXECUTABLE(${CMAKE_PROJECT_NAME} ${TARGET_SRC})
include(${PROJECT_SOURCE_DIR}/config/link.cmake)

# examples
SET(TARGET_DEFAULT_PREFIX "example_")

#osganimationviewer
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/examples/osganimationviewer)
SET(TARGET_NAME "osganimationviewer")
SET(CMAKE_PROJECT_NAME "${TARGET_DEFAULT_PREFIX}${TARGET_NAME}")

SET(TARGET_SRC
  ${SRC_DIR}/AnimtkViewer.cpp
  ${SRC_DIR}/AnimtkViewerKeyHandler.cpp
  ${SRC_DIR}/AnimtkViewerGUI.cpp
  )

ADD_EXECUTABLE(${CMAKE_PROJECT_NAME} ${TARGET_SRC})
include(${PROJECT_SOURCE_DIR}/config/link.cmake)

#osgclip
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/examples/osgclip)
SET(TARGET_NAME "osgclip")
SET(CMAKE_PROJECT_NAME "${TARGET_DEFAULT_PREFIX}${TARGET_NAME}")

SET(TARGET_SRC ${SRC_DIR}/osgclip.cpp )

ADD_EXECUTABLE(${CMAKE_PROJECT_NAME} ${TARGET_SRC})
include(${PROJECT_SOURCE_DIR}/config/link.cmake)

# osgstaticviewer
set(SRC_DIR ${OpenSceneGraph_SOURCE_DIR}/examples/osgstaticviewer)
SET(TARGET_NAME "osgstaticviewer")
SET(CMAKE_PROJECT_NAME "${TARGET_DEFAULT_PREFIX}${TARGET_NAME}")

ADD_DEFINITIONS(-DUSE_FREETYPE)

SET(TARGET_SRC ${SRC_DIR}/osgstaticviewer.cpp )

ADD_EXECUTABLE(${CMAKE_PROJECT_NAME} ${TARGET_SRC})
include(${PROJECT_SOURCE_DIR}/config/link.cmake)

#osgwindows
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/examples/osgwindows)
SET(TARGET_NAME "osgwindows")
SET(CMAKE_PROJECT_NAME "${TARGET_DEFAULT_PREFIX}${TARGET_NAME}")

SET(TARGET_SRC ${SRC_DIR}/osgwindows.cpp )

ADD_EXECUTABLE(${CMAKE_PROJECT_NAME} ${TARGET_SRC})
include(${PROJECT_SOURCE_DIR}/config/link.cmake)
