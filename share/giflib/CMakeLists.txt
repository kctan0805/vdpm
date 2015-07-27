set(CMAKE_CURRENT_SOURCE_DIR ${GIFLIB_SOURCE_DIR}/lib)

include_directories(
    ${GETOPT_INCLUDE_DIR}
    ${GIFLIB_INCLUDE_DIR}
)

set(include_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/gif_lib.h
)
set(libgif_la_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/dgif_lib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/egif_lib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/gif_font.c
    ${CMAKE_CURRENT_SOURCE_DIR}/gif_hash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/gif_hash.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gifalloc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/gif_err.c
    ${CMAKE_CURRENT_SOURCE_DIR}/gif_lib_private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/quantize.c
)

add_library(gif STATIC ${include_HEADERS} ${libgif_la_SOURCES})
