///////////////////////////////////////////////////////////////////////////////
// ViewFormGL.cpp
// ==============
// View component of OpenGL dialog window
//
//  AUTHORL Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2008-10-02
// UPDATED: 2012-06-08
///////////////////////////////////////////////////////////////////////////////

#include <sstream>
#include <iomanip>
#include <cmath>
#include "ViewFormGL.h"
#include "resource.h"
#include "Log.h"
#include "wcharUtil.h"
using namespace Win;

const int   TAU_LOW = 0;
const int   TAU_HIGH = 100;
const float TAU_SCALE = 0.001f;
const int   AFACES_LOW = 2;
const int   AFACES_HIGH = INT_MAX;
const int   AFACES_SCALE = 100;
const int   GTIME_LOW = 2;
const int   GTIME_HIGH = 72;
const int   GTIME_SCALE = 1;
const int   AMORTIZE_LOW = 1;
const int   AMORTIZE_HIGH = 8;
const int   AMORTIZE_SCALE = 1;

///////////////////////////////////////////////////////////////////////////////
// default ctor
///////////////////////////////////////////////////////////////////////////////
ViewFormGL::ViewFormGL(ModelGL* model) : model(model), parentHandle(0)
{
}


///////////////////////////////////////////////////////////////////////////////
// default dtor
///////////////////////////////////////////////////////////////////////////////
ViewFormGL::~ViewFormGL()
{
}



///////////////////////////////////////////////////////////////////////////////
// initialize all controls
///////////////////////////////////////////////////////////////////////////////
void ViewFormGL::initControls(HWND handle)
{
    // remember the handle to parent window
    parentHandle = handle;

    // set all controls
    radioOrthographic.set(handle, IDC_RADIO_ORTHO);
    radioPerspective.set(handle, IDC_RADIO_PERSPECTIVE);
    radioPerspective.check();

    radioFill.set(handle, IDC_RADIO_FILL);
    radioLine.set(handle, IDC_RADIO_LINE);
    radioPoint.set(handle, IDC_RADIO_POINT);
    radioFill.check();

    buttonReset.set(handle, IDC_BUTTON_RESET);
    editTau.set(handle, IDC_EDIT_TAU);
    editAFaces.set(handle, IDC_EDIT_AFACES);
    editGTime.set(handle, IDC_EDIT_GTIME);
    editAmortize.set(handle, IDC_EDIT_AMORTIZE);
    textStatus.set(handle, IDC_STATUS);

    spinTau.set(handle, IDC_SPIN_TAU);
    spinTau.setRange(TAU_LOW, TAU_HIGH);
    spinTau.setPos(10);
    editTau.setText(toWchar(spinTau.getPos() * TAU_SCALE));

    spinAFaces.set(handle, IDC_SPIN_AFACES);
    spinAFaces.setRange(AFACES_LOW, AFACES_HIGH);
    spinAFaces.setPos(100);
    editAFaces.setText(toWchar(spinAFaces.getPos() * AFACES_SCALE));

    spinGTime.set(handle, IDC_SPIN_GTIME);
    spinGTime.setRange(GTIME_LOW, GTIME_HIGH);
    spinGTime.setPos(8);
    editGTime.setText(toWchar(spinGTime.getPos() * GTIME_SCALE));

    spinAmortize.set(handle, IDC_SPIN_AMORTIZE);
    spinAmortize.setRange(AMORTIZE_LOW, AMORTIZE_HIGH);
    spinAmortize.setPos(1);
    editAmortize.setText(toWchar(spinAmortize.getPos() * AMORTIZE_SCALE));

    // textboxes of OpenGL functions
    textStatus.setFont(L"Courier New", 9);

    checkColor.set(handle, IDC_CHECK_COLOR);
    checkTexture.set(handle, IDC_CHECK_TEXTURE);
    checkLighting.set(handle, IDC_CHECK_LIGHTING);
}



///////////////////////////////////////////////////////////////////////////////
// change updown position
///////////////////////////////////////////////////////////////////////////////
int ViewFormGL::changeUpDownPosition(HWND handle, int position)
{
    if(handle == spinTau.getHandle())
    {
        if(position >= TAU_LOW && position <= TAU_HIGH)
        {
            editTau.setText(toWchar(position * TAU_SCALE));
            model->setTau(position * TAU_SCALE);
        }
    }
    else if(handle == spinAFaces.getHandle())
    {
        if (position >= AFACES_LOW && position <= AFACES_HIGH)
        {
            editAFaces.setText(toWchar(position * AFACES_SCALE));
            model->setAFaces(position * AFACES_SCALE);
        }
    }
    else if(handle == spinGTime.getHandle())
    {
        if (position >= GTIME_LOW && position <= GTIME_HIGH)
        {
            editGTime.setText(toWchar(position * GTIME_SCALE));
            model->setGTime(position * GTIME_SCALE);
        }
    }
    else if(handle == spinAmortize.getHandle())
    {
        if (position >= AMORTIZE_LOW && position <= AMORTIZE_HIGH)
        {
            editAmortize.setText(toWchar(position * AMORTIZE_SCALE));
            model->setAmortize(position * AMORTIZE_SCALE);
        }
    }
    return 0;
}


///////////////////////////////////////////////////////////////////////////////
// update status
///////////////////////////////////////////////////////////////////////////////
void ViewFormGL::updateStatus()
{
    std::wstringstream wss;
    const Point& pos = model->getViewPosition();
    unsigned int afaceCount = model->getAFaces();
    unsigned int vmorphCount = model->getVMorphCount();
    unsigned int tstripCount = model->getTStripCount();

    // convert number to string with limited decimal points
    //wss << std::fixed << std::setprecision(5);

    wss.str(L""); // clear
    wss << L"Tau: " << model->getTau()
        << L"\nActive faces: " << afaceCount
        << L"\nVMorphs: " << vmorphCount
        << L"\nTriangle Strips: " << tstripCount
        << L"\nAvg. faces per strip: " << ((tstripCount > 0) ? ((float)afaceCount / tstripCount) : 0)
        << L"\nFPS: " << model->getFps()
        << L"\nView position: " << pos.x << " " << pos.y << " " << pos.z
        << std::ends;

    textStatus.setText(wss.str().c_str());
}

void ViewFormGL::reset()
{
    spinTau.setPos(0);
    editTau.setText(toWchar(spinTau.getPos() * TAU_SCALE));
    model->setTau(spinTau.getPos() * TAU_SCALE);

    spinAFaces.setPos(100);
    editAFaces.setText(toWchar(spinAFaces.getPos() * AFACES_SCALE));
    model->setAFaces(spinAFaces.getPos() * AFACES_SCALE);

    spinGTime.setPos(8);
    editGTime.setText(toWchar(spinGTime.getPos() * GTIME_SCALE));
    model->setGTime(spinGTime.getPos() * GTIME_SCALE);

    spinAmortize.setPos(1);
    editAmortize.setText(toWchar(spinAmortize.getPos() * AMORTIZE_SCALE));
    model->setAmortize(spinAmortize.getPos() * AMORTIZE_SCALE);
}
