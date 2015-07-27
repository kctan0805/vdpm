///////////////////////////////////////////////////////////////////////////////
// ControllerMain.cpp
// ==================
// Derived Controller class for main window
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2006-07-09
// UPDATED: 2014-01-01
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <commctrl.h>                   // common controls
#include <sstream>
#include "ControllerMain.h"
#include "resource.h"
#include "Log.h"
using namespace Win;


// handle events(messages) on all child windows that belong to the parent window.
// For example, close all child windows when the parent got WM_CLOSE message.
// lParam can be used to specify a event or message.
bool CALLBACK enumerateChildren(HWND childHandle, LPARAM lParam);



ControllerMain::ControllerMain() : glHandle(0), formHandle(0)
{
}



int ControllerMain::command(int id, int cmd, LPARAM msg)
{
    switch(id)
    {
    case ID_FILE_EXIT:
        ::PostMessage(handle, WM_CLOSE, 0, 0);
        break;

    }

    return 0;
}



int ControllerMain::close()
{
    Win::log(""); // blank line
    Win::log("Terminating application...");

    // close all child windows first
    ::EnumChildWindows(handle, (WNDENUMPROC)enumerateChildren, (LPARAM)WM_CLOSE);

    ::DestroyWindow(handle);    // close itself
    return 0;
}



int ControllerMain::destroy()
{
    ::PostQuitMessage(0);       // exit the message loop
 
    Win::log("Main window is destroyed.");
    return 0;
}



int ControllerMain::create()
{
    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle WM_SIZE
// the width and height are for the client area
///////////////////////////////////////////////////////////////////////////////
int ControllerMain::size(int width, int height, WPARAM wParam)
{
    RECT rect;

    // get height of status bar
    HWND statusHandle = ::GetDlgItem(handle, IDC_STATUSBAR);
    ::GetWindowRect(statusHandle, &rect);
    int statusHeight = rect.bottom - rect.top;

    // get height of glDialog
    ::GetWindowRect(formHandle, &rect);
    int formHeight = rect.bottom - rect.top;

    // resize the height of glWin and reposition glDialog & status bar
    int glHeight = height - formHeight - statusHeight;
    ::SetWindowPos(glHandle, 0, 0, 0, width, glHeight, SWP_NOZORDER);
    ::SetWindowPos(formHandle, 0, 0, glHeight, width, formHeight, SWP_NOZORDER);
    ::InvalidateRect(formHandle, 0, TRUE);      // force to repaint
    ::SendMessage(statusHandle, WM_SIZE, 0, 0); // automatically resize width, so send 0s
    ::InvalidateRect(statusHandle, 0, FALSE);   // force to repaint

    // display OpenGL window dimension on the status bar
    std::wstringstream wss;
    wss << "Window Size (Client Area): " << width << " x " << height;
    ::SendMessage(statusHandle, SB_SETTEXT, 0, (LPARAM)wss.str().c_str());

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// enumerate all child windows
///////////////////////////////////////////////////////////////////////////////
bool CALLBACK enumerateChildren(HWND handle, LPARAM lParam)
{
    if(lParam == WM_CLOSE)
    {
        ::SendMessage(handle, WM_CLOSE, 0, 0);      // close child windows
    }

    return true;
}
