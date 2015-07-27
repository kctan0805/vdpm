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

const int   UPDOWN_LOW = -100;
const int   UPDOWN_HIGH = 100;
const float SCALE = 0.1f;


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
    editLeft.set(handle, IDC_EDIT_LEFT);
    editRight.set(handle, IDC_EDIT_RIGHT);
    editBottom.set(handle, IDC_EDIT_BOTTOM);
    editTop.set(handle, IDC_EDIT_TOP);
    editNear.set(handle, IDC_EDIT_NEAR);
    editFar.set(handle, IDC_EDIT_FAR);
    textGL.set(handle, IDC_GL);

    spinLeft.set(handle, IDC_SPIN_LEFT);
    spinLeft.setRange(UPDOWN_LOW, UPDOWN_HIGH);
    spinLeft.setPos(0);
    editLeft.setText(toWchar(spinLeft.getPos() * SCALE));

    spinRight.set(handle, IDC_SPIN_RIGHT);
    spinRight.setRange(UPDOWN_LOW, UPDOWN_HIGH);
    spinRight.setPos(0);
    editRight.setText(toWchar(spinRight.getPos() * SCALE));

    spinBottom.set(handle, IDC_SPIN_BOTTOM);
    spinBottom.setRange(UPDOWN_LOW, UPDOWN_HIGH);
    spinBottom.setPos(0);
    editBottom.setText(toWchar(spinBottom.getPos() * SCALE));

    spinTop.set(handle, IDC_SPIN_TOP);
    spinTop.setRange(UPDOWN_LOW, UPDOWN_HIGH);
    spinTop.setPos(0);
    editTop.setText(toWchar(spinTop.getPos() * SCALE));

    spinNear.set(handle, IDC_SPIN_NEAR);
    spinNear.setRange(1, UPDOWN_HIGH);
    spinNear.setPos(0);
    editNear.setText(toWchar(spinNear.getPos() * SCALE));

    spinFar.set(handle, IDC_SPIN_FAR);
    spinFar.setRange(1, UPDOWN_HIGH);
    spinFar.setPos(0);
    editFar.setText(toWchar(spinFar.getPos() * SCALE));

    // elements of projection matrix
    m[0].set(handle, IDC_M0);
    m[1].set(handle, IDC_M1);
    m[2].set(handle, IDC_M2);
    m[3].set(handle, IDC_M3);
    m[4].set(handle, IDC_M4);
    m[5].set(handle, IDC_M5);
    m[6].set(handle, IDC_M6);
    m[7].set(handle, IDC_M7);
    m[8].set(handle, IDC_M8);
    m[9].set(handle, IDC_M9);
    m[10].set(handle, IDC_M10);
    m[11].set(handle, IDC_M11);
    m[12].set(handle, IDC_M12);
    m[13].set(handle, IDC_M13);
    m[14].set(handle, IDC_M14);
    m[15].set(handle, IDC_M15);

    // textboxes of OpenGL functions
    textGL.setFont(L"Courier New", 9);
}



///////////////////////////////////////////////////////////////////////////////
// change updown position
///////////////////////////////////////////////////////////////////////////////
int ViewFormGL::changeUpDownPosition(HWND handle, int position)
{
    if(handle == spinLeft.getHandle())
    {
        if(position >= UPDOWN_LOW && position <= UPDOWN_HIGH)
        {
            editLeft.setText(toWchar(position * SCALE));
            model->setProjectionLeft(position * SCALE);
        }
    }
    else if(handle == spinRight.getHandle())
    {
        if(position >= UPDOWN_LOW && position <= UPDOWN_HIGH)
        {
            editRight.setText(toWchar(position * SCALE));
            model->setProjectionRight(position * SCALE);
        }
    }
    else if(handle == spinBottom.getHandle())
    {
        if(position >= UPDOWN_LOW && position <= UPDOWN_HIGH)
        {
            editBottom.setText(toWchar(position * SCALE));
            model->setProjectionBottom(position * SCALE);
        }
    }
    else if(handle == spinTop.getHandle())
    {
        if(position >= UPDOWN_LOW && position <= UPDOWN_HIGH)
        {
            editTop.setText(toWchar(position * SCALE));
            model->setProjectionTop(position * SCALE);
        }
    }
    else if(handle == spinNear.getHandle())
    {
        if(position >= 1 && position <= UPDOWN_HIGH)
        {
            editNear.setText(toWchar(position));
            model->setProjectionNear((float)position);
        }
    }
    else if(handle == spinFar.getHandle())
    {
        if(position >= 1 && position <= UPDOWN_HIGH)
        {
            editFar.setText(toWchar(position));
            model->setProjectionFar((float)position);
        }
    }

    updateProjectionMatrix();
    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// set projection matrix entries
///////////////////////////////////////////////////////////////////////////////
void ViewFormGL::setProjection(float l, float r, float b, float t, float n, float f)
{
    spinLeft.setPos((int)floor(l / SCALE));
    editLeft.setText(toWchar(spinLeft.getPos()*SCALE));

    spinRight.setPos((int)floor(r / SCALE));
    editRight.setText(toWchar(spinRight.getPos()*SCALE));

    spinBottom.setPos((int)floor(b / SCALE));
    editBottom.setText(toWchar(spinBottom.getPos()*SCALE));

    spinTop.setPos((int)floor(t / SCALE));
    editTop.setText(toWchar(spinTop.getPos()*SCALE));

    spinNear.setPos((int)n);
    editNear.setText(toWchar(spinNear.getPos()));

    spinFar.setPos((int)f);
    editFar.setText(toWchar(spinFar.getPos()));

    updateProjectionMatrix();
}



///////////////////////////////////////////////////////////////////////////////
// update Projection Matrix
///////////////////////////////////////////////////////////////////////////////
void ViewFormGL::updateProjectionMatrix()
{
    std::wstringstream wss;
    int i;

    // convert number to string with limited decimal points
    wss << std::fixed << std::setprecision(2);

    const float* matrix = model->getProjectionMatrixElements();
    for(i = 0; i < 16; ++i)
    {
        wss.str(L"");
        wss << matrix[i] << std::ends;
        m[i].setText(wss.str().c_str());
    }

    // unset floating format
    wss << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);

    // update OpenGL function calls
    //wss << std::fixed << std::setprecision(3);
    wss.str(L""); // clear
    wss << L"glMatrixMode(GL_PROJECTION);\n"
        << L"glLoadIdentity();\n";

    if(radioPerspective.isChecked())
    {
        wss << L"glFrustum(";
    }
    else
    {
        wss << L"glOrtho(";
    }

    wss << model->getProjectionLeft() << L", "
        << model->getProjectionRight() << L", "
        << model->getProjectionBottom() << L", "
        << model->getProjectionTop() << L", "
        << model->getProjectionNear() << L", "
        << model->getProjectionFar() << L");\n"
        << std::ends;

    textGL.setText(wss.str().c_str());
}

