///////////////////////////////////////////////////////////////////////////////
// ControllerGL.h
// ==============
// Derived Controller class for OpenGL window
// It is the controller of OpenGL rendering window. It initializes DC and RC,
// when WM_CREATE called, then, start new thread for OpenGL rendering loop.
//
// When this class is constructed, it gets the pointers to model and view
// components.
//
//  AUTHOR: Song Ho Ahn (song.ahn@gamil.com)
// CREATED: 2008-09-15
// UPDATED: 2013-03-01
///////////////////////////////////////////////////////////////////////////////

#ifndef WIN_CONTROLLER_GL_H
#define WIN_CONTROLLER_GL_H

#include "Controller.h"
#include "ViewGL.h"
#include "ModelGL.h"


namespace Win
{
    class ControllerGL : public Controller
    {
    public:
        ControllerGL(ModelGL* model, ViewGL* view);
        ~ControllerGL() {};

        int command(int id, int cmd, LPARAM msg);   // for WM_COMMAND
        int create();                               // create RC for OpenGL window and start new thread for rendering
        int destroy();                              // close the RC and destroy OpenGL window
        int paint();
        int lButtonDown(WPARAM state, int x, int y);
        int lButtonUp(WPARAM state, int x, int y);
        int rButtonDown(WPARAM state, int x, int y);
        int rButtonUp(WPARAM state, int x, int y);
        int mouseMove(WPARAM state, int x, int y);
        int mouseWheel(int state, int delta, int x, int y); // for WM_MOUSEWHEEL:state, delta, x, y
        int size(int w, int h, WPARAM wParam);      // for WM_SIZE: width, height, type(SIZE_MAXIMIZED...)
        int notify(int id, LPARAM lParam);          // for WM_NOTIFY


    private:
        static void threadFunction(void* arg);      // static thread function, it will route to member function, runThread()
        void runThread();                           // thread for OpenGL rendering
        void fileOpen();
        void fileClose();

        ModelGL* model;                             // pointer to model component
        ViewGL* view;                               // pointer to view component
        HANDLE threadHandle;
        unsigned int threadId;
        volatile bool loopFlag;                     // rendering loop flag
    };
}

#endif
