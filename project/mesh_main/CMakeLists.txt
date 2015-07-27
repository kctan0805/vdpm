include_directories(${MESH_INCLUDE_DIR})

set(SRCS
    Mesh-1.13/mesh.cpp
    ${MESH_SOURCE_DIR}/mesh.h
)

add_executable(mesh_main ${SRCS})
include(${PROJECT_SOURCE_DIR}/config/link.cmake)
