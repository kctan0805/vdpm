set(CMAKE_CURRENT_SOURCE_DIR ${MESH_SOURCE_DIR})
include_directories(${MESH_INCLUDE_DIR})

set(MESH_C_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/colormap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/compute_error.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mesh_run.c
    ${CMAKE_CURRENT_SOURCE_DIR}/model_analysis.c
    ${CMAKE_CURRENT_SOURCE_DIR}/reporting.c
    ${CMAKE_CURRENT_SOURCE_DIR}/xalloc.c
)

set(MESH_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/colormap.h
    ${CMAKE_CURRENT_SOURCE_DIR}/compute_error.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mesh_run.h
    ${CMAKE_CURRENT_SOURCE_DIR}/model_analysis.h
    ${CMAKE_CURRENT_SOURCE_DIR}/reporting.h
    ${CMAKE_CURRENT_SOURCE_DIR}/xalloc.h
)

set(CMAKE_CURRENT_SOURCE_DIR ${MESH_SOURCE_DIR}/lib3d/src)
set(LIB3D_C_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/geomutils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/model_in.c
    ${CMAKE_CURRENT_SOURCE_DIR}/model_in_raw.c
    ${CMAKE_CURRENT_SOURCE_DIR}/model_in_smf.c
	${CMAKE_CURRENT_SOURCE_DIR}/model_in_ply.c
	${CMAKE_CURRENT_SOURCE_DIR}/model_in_vrml_iv.c
	${CMAKE_CURRENT_SOURCE_DIR}/model_in_off.c
	${CMAKE_CURRENT_SOURCE_DIR}/block_list.c
)

set(CMAKE_CURRENT_SOURCE_DIR ${MESH_SOURCE_DIR}/lib3d/include)
set(LIB3D_INCLUDES
	${CMAKE_CURRENT_SOURCE_DIR}/3dmodel.h
	${CMAKE_CURRENT_SOURCE_DIR}/geomutils.h
	${CMAKE_CURRENT_SOURCE_DIR}/model_in.h
	${CMAKE_CURRENT_SOURCE_DIR}/model_in_ply.h
	${CMAKE_CURRENT_SOURCE_DIR}/types.h
	${CMAKE_CURRENT_SOURCE_DIR}/block_list.h
	${CMAKE_CURRENT_SOURCE_DIR}/debug_print.h
)

add_library(mesh STATIC ${MESH_C_SRCS} ${MESH_INCLUDES} ${LIB3D_C_SRCS} ${LIB3D_INCLUDES})
