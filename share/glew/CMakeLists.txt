set(CMAKE_CURRENT_SOURCE_DIR ${GLEW_SOURCE_DIR})
set(GLEW_VERSION "1.11.0")

add_definitions(-DGLEW_BUILD -DGLEW_NO_GLU)

include_directories( ${GLEW_INCLUDE_DIR} )

add_library(glew STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/glew.c )
