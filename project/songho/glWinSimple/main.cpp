///////////////////////////////////////////////////////////////////////////////
// main.cpp
// ========
// main driver
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2006-06-28
// UPDATED: 2006-07-08
///////////////////////////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN             // exclude rarely-used stuff from Windows headers

#include <windows.h>
#include "Window.h"
#include "ControllerGL.h"
#include "ModelGL.h"
#include "ViewGL.h"


// function declarations
int mainMessageLoop(HACCEL hAccelTable=0);




///////////////////////////////////////////////////////////////////////////////
// main function of a windows application
///////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdArgs, int cmdShow)
{
    // instantiate model and view components, so "controller" component can reference them
    ModelGL model;
    Win::ViewGL view;   // under "Win" namespace because it is Windows specific view component.

    // create "controller" component by specifying what are "model" and "view"
    Win::ControllerGL glCtrl(&model, &view);

    // create window with given controller
    Win::Window glWin(hInst, L"glWinSimple", 0, &glCtrl);
    glWin.setWindowStyle(WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
    glWin.setClassStyle(CS_OWNDC);
    glWin.setWidth(400);
    glWin.setHeight(300);

    glWin.create();
    glWin.show();

    // main message loop //////////////////////////////////////////////////////
    int exitCode;
    exitCode = mainMessageLoop();

    return exitCode;
}



///////////////////////////////////////////////////////////////////////////////
// main message loop
///////////////////////////////////////////////////////////////////////////////
int mainMessageLoop(HACCEL hAccelTable)
{
    MSG msg;

    while(::GetMessage(&msg, 0, 0, 0) > 0)  // loop until WM_QUIT(0) received
    {
        // now, handle window messages
        if(!::TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;                 // return nExitCode of PostQuitMessage()
}
