///////////////////////////////////////////////////////////////////////////////
// ControllerFormGL.cpp
// ====================
// Derived Controller class for OpenGL dialog window
//
//  AUTHOR: Song Ho Ahn (song.ahn@gamil.com)
// CREATED: 2008-10-02
// UPDATED: 2013-03-17
///////////////////////////////////////////////////////////////////////////////

#include <process.h>                                // for _beginthreadex()
#include "ControllerFormGL.h"
#include "resource.h"
#include "Log.h"
using namespace Win;



///////////////////////////////////////////////////////////////////////////////
// default contructor
///////////////////////////////////////////////////////////////////////////////
ControllerFormGL::ControllerFormGL(ModelGL* model, ViewFormGL* view) : model(model), view(view)
{
}



///////////////////////////////////////////////////////////////////////////////
// handle WM_CREATE
///////////////////////////////////////////////////////////////////////////////
int ControllerFormGL::create()
{
    // initialize all controls
    view->initControls(handle);

    SetTimer(handle, 1, 1000, NULL);

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle WM_COMMAND
///////////////////////////////////////////////////////////////////////////////
int ControllerFormGL::command(int id, int command, LPARAM msg)
{
    switch(id)
    {
    case IDC_BUTTON_RESET:
        if(command == BN_CLICKED)
        {
            view->reset();
        }
        break;
    case IDC_RADIO_ORTHO:
        if(command == BN_CLICKED)
        {
            model->setProjectionMode(1);
        }
        break;
    case IDC_RADIO_PERSPECTIVE:
        if(command == BN_CLICKED)
        {
            model->setProjectionMode(0);
        }
        break;
    case IDC_RADIO_FILL:
        if(command == BN_CLICKED)
        {
            model->setDrawMode(0);
        }
        break;
    case IDC_RADIO_LINE:
        if(command == BN_CLICKED)
        {
            model->setDrawMode(1);
        }
        break;
    case IDC_RADIO_POINT:
        if(command == BN_CLICKED)
        {
            model->setDrawMode(2);
        }
        break;
    case IDC_CHECK_COLOR:
        if (command == BN_CLICKED)
        {
            if (view->checkColor.isChecked())
                model->enableColor();
            else
                model->disableColor();
        }
        break;
    case IDC_CHECK_TEXTURE:
        if (command == BN_CLICKED)
        {
            if (view->checkTexture.isChecked())
                model->enableTexture();
            else
                model->disableTexture();
        }
        break;
    case IDC_CHECK_LIGHTING:
        if (command == BN_CLICKED)
        {
            if (view->checkLighting.isChecked())
                model->enableLighting();
            else
                model->disableLighting();
        }
        break;
    }

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle WM_NOTIFY
// The id is not guaranteed to be unique, so use NMHDR.hwndFrom and NMHDR.idFrom.
///////////////////////////////////////////////////////////////////////////////
int ControllerFormGL::notify(int id, LPARAM lParam)
{
    // first cast lParam to NMHDR* to know what the control is
    NMHDR* nmhdr = (NMHDR*)lParam;
    HWND from = nmhdr->hwndFrom;
    NMUPDOWN* nmUpDown = 0;

    switch(nmhdr->code)
    {
    // UpDownBox notifications =========
    case UDN_DELTAPOS:         // the change of position has begun
        // cast again lParam to NMUPDOWN*
        nmUpDown = (NMUPDOWN*)lParam;
        return view->changeUpDownPosition(from, nmUpDown->iPos + nmUpDown->iDelta);
        break;

    default:
        break;
    }

    // handled notifications
    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle WM_TIMER notification
///////////////////////////////////////////////////////////////////////////////
int ControllerFormGL::timer(WPARAM eventId, LPARAM callback)
{
    view->updateStatus();
    SetTimer(handle, 1, 1000, NULL);
    return 0;
}



