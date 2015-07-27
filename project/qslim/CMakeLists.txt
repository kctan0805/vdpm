include_directories(${GETOPT_INCLUDE_DIR} ${GFX_INCLUDE_DIR} ${FLTK_INCLUDE_DIR} ${MIXKIT_INCLUDE_DIR})

set(SRCS
    qslim/cmdline.cxx
    qslim/main.cxx
    qslim/output.cxx
    qslim/qslim.cxx
)
set(QVIS_SRCS
    qslim/cmdline.cxx
    qslim/main.cxx
    qslim/output.cxx
    qslim/qvis.cxx
)

add_executable(qslim ${SRCS})
include(${PROJECT_SOURCE_DIR}/config/link.cmake)

set(CMAKE_PROJECT_NAME qvis)
add_executable(qvis ${QVIS_SRCS})
include(${PROJECT_SOURCE_DIR}/config/link.cmake)

set(CMAKE_PROJECT_NAME propslim)
add_executable(propslim examples/propslim.cxx)
include(${PROJECT_SOURCE_DIR}/config/link.cmake)

set(CMAKE_PROJECT_NAME smfview)
add_executable(smfview examples/smfview.cxx)
include(${PROJECT_SOURCE_DIR}/config/link.cmake)

set(CMAKE_PROJECT_NAME propvis)
add_executable(propvis examples/propvis.cxx)
include(${PROJECT_SOURCE_DIR}/config/link.cmake)
