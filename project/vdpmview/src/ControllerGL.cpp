///////////////////////////////////////////////////////////////////////////////
// ControllerGL.cpp
// ================
// Derived Controller class for OpenGL window
// It is the controller of OpenGL rendering window. It initializes DC and RC,
// when WM_CREATE called, then, start new thread for OpenGL rendering loop.
//
// When this class is constructed, it gets the pointers to model and view
// components.
//
//  AUTHOR: Song Ho Ahn (song.ahn@gamil.com)
// CREATED: 2008-09-15
// UPDATED: 2013-03-17
///////////////////////////////////////////////////////////////////////////////

#include <process.h>                                // for _beginthreadex()
#include <windows.h>
#include <Commdlg.h>
#include <string.h>
#include <GL/glew.h>
#include "ControllerGL.h"
#include "resource.h"
#include "Log.h"
using namespace Win;



///////////////////////////////////////////////////////////////////////////////
// default contructor
///////////////////////////////////////////////////////////////////////////////
ControllerGL::ControllerGL(ModelGL* model, ViewGL* view) : model(model), view(view),
                                                           threadHandle(0), threadId(0),
                                                           loopFlag(false)
{
}



///////////////////////////////////////////////////////////////////////////////
// handle WM_DESTROY
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::destroy()
{
    loopFlag = false;
    ::WaitForSingleObject(threadHandle, INFINITE);   // wait until rendering thread is terminated

    // close OpenGL rendering context (RC)
    view->closeContext(handle);

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle WM_CREATE
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::create()
{
    // create a OpenGL rendering context
    if(!view->createContext(handle, 32, 24, 8))
    {
        Win::log(L"[ERROR] Failed to create OpenGL rendering context from ControllerGL::create().");
        return -1;
    }

    // create a thread for OpenGL rendering
    // The params of _beginthreadex() are security, stackSize, functionPtr, argPtr, initFlag, threadId.
    threadHandle = (HANDLE)_beginthreadex(0, 0, (unsigned (__stdcall *)(void *))threadFunction, this, 0, &threadId);
    if(threadHandle)
    {
        loopFlag = true;
        Win::log(L"Created a rendering thread for OpenGL.");
    }
    else
    {
        Win::log(L"[ERROR] Failed to create rendering thread from ControllerGL::create().");
    }

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle WM_PAINT
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::paint()
{
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// handle WM_COMMAND
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::command(int id, int cmd, LPARAM msg)
{
    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// route to worker thread
// The function prototype is:
// unsigned int (__stdcall *)(void *)
///////////////////////////////////////////////////////////////////////////////
void ControllerGL::threadFunction(void* param)
{
    ((ControllerGL*)param)->runThread();
}



///////////////////////////////////////////////////////////////////////////////
// rendering thread
// initialize OpenGL states and start rendering loop
///////////////////////////////////////////////////////////////////////////////
void ControllerGL::runThread()
{
    // set the current RC in this thread
    ::wglMakeCurrent(view->getDC(), view->getRC());

    if (GLEW_OK != glewInit())
    {
        Win::log(L"[ERROR] glewInit failed\n");
        return;
    }

    // initialize OpenGL states
    model->init();
    Win::log(L"Initialized OpenGL states.");

    // rendering loop
    Win::log(L"Entering OpenGL rendering thread...");
    while(loopFlag)
    {
        ::Sleep(33);                   // yield to other processes or threads
        model->draw();
        view->swapBuffers();
    }

    // terminate rendering thread
    ::wglMakeCurrent(0, 0);             // unset RC
    ::CloseHandle(threadHandle);
    Win::log(L"Exit OpenGL rendering thread.");
}



///////////////////////////////////////////////////////////////////////////////
// handle Left mouse down
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::lButtonDown(WPARAM state, int x, int y)
{
    // update mouse position
    model->setMousePosition(x, y);
    //model->testVsplit();

    // set focus to receive wm_mousewheel event
    ::SetFocus(handle);

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle Left mouse up
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::lButtonUp(WPARAM state, int x, int y)
{
    // update mouse position
    model->setMousePosition(x, y);
    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle reft mouse down
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::rButtonDown(WPARAM state, int x, int y)
{
    // update mouse position
    model->setMousePosition(x, y);
    //model->testEcol();

    // set focus to receive wm_mousewheel event
    ::SetFocus(handle);

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle reft mouse up
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::rButtonUp(WPARAM state, int x, int y)
{
    // update mouse position
    model->setMousePosition(x, y);
    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle WM_MOUSEMOVE
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::mouseMove(WPARAM state, int x, int y)
{
    if(state == MK_LBUTTON)
    {
        model->rotateCamera(x, y);
    }
    if(state == MK_RBUTTON)
    {
        model->zoomCamera(y);
    }

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle WM_MOUSEWHEEL
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::mouseWheel(int state, int delta, int x, int y)
{
    model->zoomCameraDelta(delta);
    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle WM_SIZE
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::size(int w, int h, WPARAM wParam)
{
    model->setWindowSize(w, h);
    Win::log(L"Changed OpenGL rendering window size: %dx%d.", w, h);
    return 0;
}


///////////////////////////////////////////////////////////////////////////////
// handle WM_NOTIFY
// The id is not guaranteed to be unique, so use NMHDR.hwndFrom and NMHDR.idFrom.
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::notify(int id, LPARAM lParam)
{
    switch (id)
    {
    case IDM_FILE_OPEN:
        fileOpen();
        break;

    case IDM_FILE_CLOSE:
        fileClose();
        break;

    default:
        break;
    }

    // handled notifications
    return 0;
}

void ControllerGL::fileOpen()
{
    char szFileTitle[_MAX_FNAME + _MAX_EXT] = "";
    char szFileLine[_MAX_PATH] = "";
    char szFilter[] = "VDPM file (*.vdpm)\0*.vdpm\0"
        "Texture file (*.bmp)\0*.bmp\0";
    OPENFILENAMEA ofn;

    ofn.lStructSize = sizeof OPENFILENAMEA;
    ofn.hwndOwner = handle;
    ofn.hInstance = (HINSTANCE)GetWindowLong(handle, GWL_HINSTANCE);
    ofn.lpstrFilter = szFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 0;
    ofn.lpstrFile = szFileLine;
    ofn.nMaxFile = sizeof szFileLine;
    ofn.lpstrFileTitle = szFileTitle;
    ofn.nMaxFileTitle = sizeof szFileTitle;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = NULL;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_READONLY;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = NULL;
    ofn.lCustData = 0;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = NULL;

    if (!GetOpenFileNameA(&ofn))
        return;

    if (!stricmp(&szFileLine[ofn.nFileExtension], "vdpm"))
    {
        model->loadSRMesh(szFileLine);
    }
    else if (!stricmp(&szFileLine[ofn.nFileExtension], "bmp"))
    {
        model->loadTexture(szFileLine);
    }
}

void ControllerGL::fileClose()
{
    model->unload();
}
