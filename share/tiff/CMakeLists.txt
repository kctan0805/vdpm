include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${JPEG_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})

set(CMAKE_CURRENT_SOURCE_DIR ${TIFF_SOURCE_DIR}/libtiff)

set(noinst_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/t4.h
	${CMAKE_CURRENT_SOURCE_DIR}/tif_dir.h
	${CMAKE_CURRENT_SOURCE_DIR}/tif_predict.h
	${CMAKE_CURRENT_SOURCE_DIR}/tiffiop.h
	${CMAKE_CURRENT_SOURCE_DIR}/uvcode.h
)
set(nodist_libtiffinclude_HEADERS
	tiffconf.h
)
set(libtiff_la_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/tif_aux.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_close.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_codec.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_color.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_compress.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_dir.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_dirinfo.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_dirread.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_dirwrite.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_dumpmode.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_error.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_extension.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_fax3.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_fax3sm.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_flush.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_getimage.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_jbig.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_jpeg.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_jpeg_12.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_luv.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_lzma.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_lzw.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_next.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_ojpeg.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_open.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_packbits.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_pixarlog.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_predict.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_print.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_read.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_strip.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_swab.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_thunder.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_tile.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_version.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_warning.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_write.c
	${CMAKE_CURRENT_SOURCE_DIR}/tif_zip.c
)

if(MSVC)
    set(libtiff_la_SOURCES ${libtiff_la_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/tif_win32.c
    )
else()
    set(libtiff_la_SOURCES ${libtiff_la_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/tif_unix.c
    )
endif()

add_library(tiff STATIC ${noinst_HEADERS} ${nodist_libtiffinclude_HEADERS} ${libtiff_la_SOURCES})
