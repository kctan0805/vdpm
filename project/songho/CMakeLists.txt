add_definitions(
    -D_UNICODE
)

# glWin
set(CMAKE_PROJECT_NAME glWin)

set(SRCS
    glWin/Bmp.cpp
    glWin/Controller.cpp
    glWin/ControllerFormGL.cpp
    glWin/ControllerGL.cpp
    glWin/ControllerMain.cpp
    glWin/DialogWindow.cpp
    glWin/Log.cpp
    glWin/main.cpp
    glWin/ModelGL.cpp
    glWin/procedure.cpp
    glWin/ViewFormGL.cpp
    glWin/ViewGL.cpp
    glWin/wcharUtil.cpp
    glWin/Window.cpp
    glWin/Bmp.h
    glWin/Controller.h
    glWin/ControllerFormGL.h
    glWin/ControllerGL.h
    glWin/ControllerMain.h
    glWin/Controls.h
    glWin/DialogWindow.h
    glWin/Log.h
    glWin/logResource.h
    glWin/ModelGL.h
    glWin/procedure.h
    glWin/resource.h
    glWin/ViewFormGL.h
    glWin/ViewGL.h
    glWin/wcharUtil.h
    glWin/Window.h
    glWin/glWin.rc
    glWin/log.rc
)

add_executable(glWin WIN32 ${SRCS})
include(${PROJECT_SOURCE_DIR}/config/link.cmake)

# glWinSimple
set(CMAKE_PROJECT_NAME glWinSimple)

set(SRCS
    glWinSimple/Controller.cpp
    glWinSimple/ControllerGL.cpp
    glWinSimple/main.cpp
    glWinSimple/ModelGL.cpp
    glWinSimple/procedure.cpp
    glWinSimple/ViewGL.cpp
    glWinSimple/Window.cpp
    glWinSimple/Controller.h
    glWinSimple/ControllerGL.h
    glWinSimple/ModelGL.h
    glWinSimple/procedure.h
    glWinSimple/ViewGL.h
    glWinSimple/Window.h
)

add_executable(glWinSimple WIN32 ${SRCS})
include(${PROJECT_SOURCE_DIR}/config/link.cmake)

# matrixModelView
set(CMAKE_PROJECT_NAME matrixModelView)

set(SRCS
    matrixModelView/Controller.cpp
    matrixModelView/ControllerFormGL.cpp
    matrixModelView/ControllerGL.cpp
    matrixModelView/ControllerMain.cpp
    matrixModelView/DialogWindow.cpp
    matrixModelView/Log.cpp
    matrixModelView/main.cpp
    matrixModelView/Matrices.cpp
    matrixModelView/ModelGL.cpp
    matrixModelView/procedure.cpp
    matrixModelView/ViewFormGL.cpp
    matrixModelView/ViewGL.cpp
    matrixModelView/wcharUtil.cpp
    matrixModelView/Window.cpp
    matrixModelView/cameraSimple.h
    matrixModelView/Controller.h
    matrixModelView/ControllerFormGL.h
    matrixModelView/ControllerGL.h
    matrixModelView/ControllerMain.h
    matrixModelView/Controls.h
    matrixModelView/DialogWindow.h
    matrixModelView/Log.h
    matrixModelView/Matrices.h
    matrixModelView/ModelGL.h
    matrixModelView/procedure.h
    matrixModelView/resource.h
    matrixModelView/teapot.h
    matrixModelView/ViewFormGL.h
    matrixModelView/ViewGL.h
    matrixModelView/wcharUtil.h
    matrixModelView/Window.h
    matrixModelView/matrixModelView.rc
    matrixModelView/songho.ico
    matrixModelView/songho_alpha.bmp
)

add_executable(matrixModelView WIN32 ${SRCS})
include(${PROJECT_SOURCE_DIR}/config/link.cmake)

# matrixProjection
set(CMAKE_PROJECT_NAME matrixProjection)

set(SRCS
    matrixProjection/Controller.cpp
    matrixProjection/ControllerFormGL.cpp
    matrixProjection/ControllerGL.cpp
    matrixProjection/ControllerMain.cpp
    matrixProjection/DialogWindow.cpp
    matrixProjection/Log.cpp
    matrixProjection/main.cpp
    matrixProjection/Matrices.cpp
    matrixProjection/ModelGL.cpp
    matrixProjection/procedure.cpp
    matrixProjection/ViewFormGL.cpp
    matrixProjection/ViewGL.cpp
    matrixProjection/wcharUtil.cpp
    matrixProjection/Window.cpp
    matrixProjection/cameraSimple.h
    matrixProjection/Controller.h
    matrixProjection/ControllerFormGL.h
    matrixProjection/ControllerGL.h
    matrixProjection/ControllerMain.h
    matrixProjection/Controls.h
    matrixProjection/DialogWindow.h
    matrixProjection/Log.h
    matrixProjection/logResource.h
    matrixProjection/Matrices.h
    matrixProjection/ModelGL.h
    matrixProjection/procedure.h
    matrixProjection/resource.h
    matrixProjection/Vectors.h
    matrixProjection/ViewFormGL.h
    matrixProjection/ViewGL.h
    matrixProjection/wcharUtil.h
    matrixProjection/Window.h
    matrixProjection/matrixProjection.rc
    matrixProjection/songho.ico
    matrixProjection/songho_alpha.bmp
)

add_executable(matrixProjection WIN32 ${SRCS})
include(${PROJECT_SOURCE_DIR}/config/link.cmake)
