set(CMAKE_CURRENT_SOURCE_DIR ${FREETYPE_SOURCE_DIR})
cmake_minimum_required(VERSION 2.6)

set(VERSION_MAJOR "2")
set(VERSION_MINOR "5")
set(VERSION_PATCH "3")
set(PROJECT_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

# Compiler definitions for building the library
add_definitions(-DFT2_BUILD_LIBRARY)

# Specify library include directories
include_directories("${FREETYPE_SOURCE_DIR}/include")

set(BASE_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/autofit/autofit.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftadvanc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftbbox.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftbitmap.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftcalc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftcid.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftdbgmem.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftdebug.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftfstype.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftgasp.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftgloadr.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftglyph.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftgxval.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftinit.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftlcdfil.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftmm.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftobjs.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftotval.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftoutln.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftpatent.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftpfr.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftrfork.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftsnames.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftstream.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftstroke.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftsynth.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftsystem.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/fttrigon.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/fttype1.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftutil.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftwinfnt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ftxf86.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/bdf/bdf.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/bzip2/ftbzip2.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cache/ftcache.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cff/cff.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cid/type1cid.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/gzip/ftgzip.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lzw/ftlzw.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pcf/pcf.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pfr/pfr.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/psaux/psaux.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pshinter/pshinter.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/psnames/psmodule.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/raster/raster.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/sfnt/sfnt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/smooth/smooth.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/truetype/truetype.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/type1/type1.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/type42/type42.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/winfonts/winfnt.c
)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/truetype")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/sfnt")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/autofit")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/smooth")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/raster")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/psaux")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/psnames")

add_library(freetype ${BASE_SRCS})
