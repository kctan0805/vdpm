///////////////////////////////////////////////////////////////////////////////
// ControllerMain.cpp
// ==================
// Derived Controller class for main window
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2006-07-09
// UPDATED: 2013-03-17
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <commctrl.h>                   // common controls
#include "ControllerMain.h"
#include "resource.h"
#include "Log.h"

using namespace Win;


bool CALLBACK enumerateChildren(HWND childHandle, LPARAM lParam);
BOOL CALLBACK aboutDialogProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


int ControllerMain::command(int id, int cmd, LPARAM msg)
{
    switch(id)
    {
    case IDM_FILE_OPEN:
    case IDM_FILE_CLOSE:
        ::PostMessage(glHandle, WM_NOTIFY, id, 0);
        break;

    case IDM_FILE_EXIT:
        // main menu: File > Exit
        ::PostMessage(handle, WM_CLOSE, 0, 0);
        break;

    case IDM_HELP_ABOUT:
        // create a modal dialog box, and destroy it in aboutDialogProcedure().
        ::DialogBox((HINSTANCE)::GetWindowLongPtr(handle, GWL_HINSTANCE), MAKEINTRESOURCE(IDD_ABOUT), handle, aboutDialogProcedure);
        break;
    }

    return 0;
}



int ControllerMain::close()
{
    Win::log("Closing the application...");
    ::DestroyWindow(handle);                    // close itself and children
    return 0;
}



int ControllerMain::destroy()
{
    ::PostQuitMessage(0);       // exit the message loop
    Win::log("Main windows is destroyed.");
    return 0;
}



int ControllerMain::create()
{
    return 0;
}



int ControllerMain::size(int w, int h, WPARAM wParam)
{
    RECT rect;

    // get client dim of mainWin
    ::GetClientRect(handle, &rect);
    int mainClientWidth = rect.right - rect.left;
    int mainClientHeight = rect.bottom - rect.top;

    // get dim of glWin
    ::GetWindowRect(glHandle, &rect);
    int glWidth = rect.right - rect.left;
    int glHeight = rect.bottom - rect.top;

    // get dim of glDialog
    ::GetWindowRect(formHandle, &rect);
    int formWidth = rect.right - rect.left;

    // resize glWin and reposition glDialog
    glHeight = mainClientHeight;
    if(glHeight % 2 != 0) glHeight--; // make it even
    glWidth = glHeight / 2 + 2;
    Win::log("W: %d, H: %d", glWidth, glHeight);
    formWidth = mainClientWidth - glWidth;
    ::SetWindowPos(glHandle, 0, 0, 0, glWidth, glHeight, SWP_NOZORDER);
    ::SetWindowPos(formHandle, 0, glWidth, 0, formWidth, glHeight, SWP_NOZORDER);
    ::InvalidateRect(formHandle, 0, TRUE);      // force to repaint

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// enumerate all child windows
///////////////////////////////////////////////////////////////////////////////
bool CALLBACK enumerateChildren(HWND handle, LPARAM lParam)
{
    if(lParam == WM_CLOSE)
    {
        ::SendMessage(handle, WM_CLOSE, 0, 0);  // close child windows
    }

    return true;
}



///////////////////////////////////////////////////////////////////////////////
// dialog procedure for About window
///////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK aboutDialogProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_COMMAND:
        if(LOWORD(wParam) == IDC_OK && HIWORD(wParam) == BN_CLICKED)
        {
            ::EndDialog(hwnd, 0);
        }
        break;
    }

    return false;
}

