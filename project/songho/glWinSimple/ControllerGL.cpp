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
// CREATED: 2006-07-09
// UPDATED: 2013-03-17
///////////////////////////////////////////////////////////////////////////////

#include <process.h>                                // for _beginthreadex()
#include "ControllerGL.h"
using namespace Win;



///////////////////////////////////////////////////////////////////////////////
// default contructor
///////////////////////////////////////////////////////////////////////////////
ControllerGL::ControllerGL(ModelGL* model, ViewGL* view) : modelGL(model), viewGL(view),
                                                           threadHandle(0), threadId(0),
                                                           loopFlag(false), resizeFlag(false),
                                                           clientWidth(0), clientHeight(0)
{
}



///////////////////////////////////////////////////////////////////////////////
// handle WM_CLOSE
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::close()
{
    loopFlag = false;
    ::WaitForSingleObject(threadHandle, INFINITE);  // wait for rendering thread is terminated

    // close OpenGL Rendering context
    viewGL->closeContext(handle);

    ::DestroyWindow(handle);
    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle WM_DESTROY
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::destroy()
{
    ::PostQuitMessage(0);       // exit the message loop
    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle WM_CREATE
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::create()
{
    // create a OpenGL rendering context
    if(!viewGL->createContext(handle, 32, 24, 8))
    {
        //Win::log(L"[ERROR] Failed to create OpenGL rendering context from ControllerGL::create().");
        return -1;
    }

    // create a thread for OpenGL rendering
    // The params of _beginthreadex() are security, stackSize, functionPtr, argPtr, initFlag, threadId.
    threadHandle = (HANDLE)_beginthreadex(0, 0, (unsigned (__stdcall *)(void *))threadFunction, this, 0, &threadId);
    if(threadHandle)
    {
        loopFlag = true;
        //Win::log(L"Created a rendering thread for OpenGL.");
    }
    else
    {
        ;//Win::log(L"[ERROR] Failed to create rendering thread from ControllerGL::create().");
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
    ::wglMakeCurrent(viewGL->getDC(), viewGL->getRC());

    // initialize OpenGL states
    modelGL->init();

    // cofigure projection matrix
    RECT rect;
    ::GetClientRect(handle, &rect);
    modelGL->setViewport(rect.right, rect.bottom);

    // rendering loop
    while(loopFlag)
    {
        ::Sleep(10);                    // yield to other processes or threads

        if(resizeFlag)
        {
            modelGL->setViewport(clientWidth, clientHeight);
            resizeFlag = false;
        }

        modelGL->draw();
        viewGL->swapBuffers();
    }

    // terminate rendering thread
    ::wglMakeCurrent(0, 0);             // unset RC
    ::CloseHandle(threadHandle);
}



///////////////////////////////////////////////////////////////////////////////
// handle Left mouse down
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::lButtonDown(WPARAM state, int x, int y)
{
    // update mouse position
    modelGL->setMousePosition(x, y);

    if(state == MK_LBUTTON)
    {
        modelGL->setMouseLeft(true);
    }

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle Left mouse up
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::lButtonUp(WPARAM state, int x, int y)
{
    // update mouse position
    modelGL->setMousePosition(x, y);

    modelGL->setMouseLeft(false);

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle reft mouse down
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::rButtonDown(WPARAM state, int x, int y)
{
    // update mouse position
    modelGL->setMousePosition(x, y);

    if(state == MK_RBUTTON)
    {
        modelGL->setMouseRight(true);
    }

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle reft mouse up
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::rButtonUp(WPARAM state, int x, int y)
{
    // update mouse position
    modelGL->setMousePosition(x, y);

    modelGL->setMouseRight(false);

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle WM_MOUSEMOVE
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::mouseMove(WPARAM state, int x, int y)
{
    if(state == MK_LBUTTON)
    {
        modelGL->rotateCamera(x, y);
    }
    if(state == MK_RBUTTON)
    {
        modelGL->zoomCamera(y);
    }

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle WM_KEYDOWN
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::keyDown(int key, LPARAM lParam)
{
    if(key == VK_ESCAPE)
    {
        ::PostMessage(handle, WM_CLOSE, 0, 0);
    }

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle WM_SIZE notification
// Note that the input param, width and height is for client area only.
// It excludes non-client area.
///////////////////////////////////////////////////////////////////////////////
int ControllerGL::size(int width, int height, WPARAM type)
{
    resizeFlag = true;
    clientWidth = width;
    clientHeight = height;

    return 0;
}
