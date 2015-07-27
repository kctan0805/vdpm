include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GFX_INCLUDE_DIR}
    ${FLTK_INCLUDE_DIR}
    ${JPEG_INCLUDE_DIR}
    ${PNG_INCLUDE_DIR}
    ${TIFF_INCLUDE_DIR}
)

set(CMAKE_CURRENT_SOURCE_DIR ${GFX_SOURCE_DIR}/src)

set(VEC_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/mat2.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/mat3.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/mat4.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/quat.cxx
	${CMAKE_CURRENT_SOURCE_DIR}/symmat2.cxx
	${CMAKE_CURRENT_SOURCE_DIR}/symmat3.cxx
	${CMAKE_CURRENT_SOURCE_DIR}/symmat4.cxx
)
set(GEO_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/geom3d.cxx
)
set(IMG_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/raster.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/raster-pnm.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/raster-tiff.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/raster-jpeg.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/raster-png.cxx
)
set(MISC_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/time.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/script.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/color.cxx
)
set(GUI_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/gui.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/gltools.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/baseball.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/trackball.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/arcball.cxx
)

add_library(gfx STATIC ${VEC_SRCS} ${GEO_SRCS} ${IMG_SRCS} ${MISC_SRCS} ${GUI_SRCS})
