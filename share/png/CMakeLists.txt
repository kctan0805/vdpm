set(CMAKE_CURRENT_SOURCE_DIR ${PNG_SOURCE_DIR})

cmake_minimum_required(VERSION 2.4.4)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

set(PNGLIB_MAJOR 1)
set(PNGLIB_MINOR 6)
set(PNGLIB_RELEASE 14)
set(PNGLIB_NAME libpng${PNGLIB_MAJOR}${PNGLIB_MINOR})
set(PNGLIB_VERSION ${PNGLIB_MAJOR}.${PNGLIB_MINOR}.${PNGLIB_RELEASE})

# needed packages
include_directories(${ZLIB_INCLUDE_DIR})

option(PNG_STATIC "Build static lib" ON)

# Many more configuration options could be added here
if (CMAKE_BUILD_TYPE STREQUAL Debug)
option(PNG_DEBUG         "Build with debug output" NO)
else()
option(PNG_DEBUG         "Build with debug output" YES)
endif()
option(PNGARG            "Disable ANSI-C prototypes" NO)

# SET LIBNAME
set(PNG_LIB_NAME png)

# Use the prebuilt pnglibconf.h file from the scripts folder
# TODO: fix this by building with awk; without this no cmake build can be
# configured directly (to do so indirectly use your local awk to build a
# pnglibconf.h in the build directory.)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/pnglibconf.h.prebuilt
               ${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# OUR SOURCES
set(libpng_public_hdrs
  ${CMAKE_CURRENT_SOURCE_DIR}/png.h
  ${CMAKE_CURRENT_SOURCE_DIR}/pngconf.h
  ${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.h
)
set(libpng_sources
  ${libpng_public_hdrs}
  ${CMAKE_CURRENT_SOURCE_DIR}/pngdebug.h
  ${CMAKE_CURRENT_SOURCE_DIR}/pnginfo.h
  ${CMAKE_CURRENT_SOURCE_DIR}/pngpriv.h
  ${CMAKE_CURRENT_SOURCE_DIR}/pngstruct.h
  ${CMAKE_CURRENT_SOURCE_DIR}/png.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pngerror.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pngget.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pngmem.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pngpread.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pngread.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pngrio.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pngrtran.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pngrutil.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pngset.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pngtrans.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pngwio.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pngwrite.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pngwtran.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pngwutil.c
)
# SOME NEEDED DEFINITIONS

if(PNG_DEBUG)
  add_definitions(-DPNG_DEBUG)
endif()

# NOW BUILD OUR TARGET
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${ZLIB_INCLUDE_DIR})

# does not work without changing name
add_library(${PNG_LIB_NAME} STATIC ${libpng_sources})
