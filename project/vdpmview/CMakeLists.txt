set(CMAKE_CURRENT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

include_directories(${GLEW_INCLUDE_DIR} ${VDPM_INCLUDE_DIR})

add_definitions(
    -D_UNICODE
)

set(SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/Controller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ControllerFormGL.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ControllerGL.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ControllerMain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DialogWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Log.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Matrices.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ModelGL.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/procedure.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ViewFormGL.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ViewGL.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wcharUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Window.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cameraSimple.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Controller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ControllerFormGL.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ControllerGL.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ControllerMain.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Controls.h
    ${CMAKE_CURRENT_SOURCE_DIR}/DialogWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Log.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Matrices.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ModelGL.h
    ${CMAKE_CURRENT_SOURCE_DIR}/procedure.h
    ${CMAKE_CURRENT_SOURCE_DIR}/resource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Vectors.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ViewFormGL.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ViewGL.h
    ${CMAKE_CURRENT_SOURCE_DIR}/wcharUtil.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Window.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vdpmview.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/Bmp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Bmp.h
)

add_executable(vdpmview WIN32 ${SRCS})
include(${PROJECT_SOURCE_DIR}/config/link.cmake)
