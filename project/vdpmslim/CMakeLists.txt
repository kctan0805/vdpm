include_directories(${GETOPT_INCLUDE_DIR} ${GFX_INCLUDE_DIR} ${FLTK_INCLUDE_DIR} ${MIXKIT_INCLUDE_DIR})

set(SRCS
    cmdline.cxx
    main.cxx
    output.cxx
    qslim.cxx
    qslim.h
)
set(QVIS_SRCS
    cmdline.cxx
    main.cxx
    output.cxx
    qvis.cxx
)

add_executable(vdpmslim ${SRCS})
include(${PROJECT_SOURCE_DIR}/config/link.cmake)

set(CMAKE_PROJECT_NAME vdpmvis)
add_executable(vdpmvis ${QVIS_SRCS})
include(${PROJECT_SOURCE_DIR}/config/link.cmake)
