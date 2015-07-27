///////////////////////////////////////////////////////////////////////////////
// ViewFormGL.cpp
// ==============
// View component of OpenGL dialog window
//
//  AUTHORL Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2008-09-15
// UPDATED: 2011-03-24
///////////////////////////////////////////////////////////////////////////////

#include <sstream>
#include <iomanip>
#include "ViewFormGL.h"
#include "resource.h"
#include "Log.h"
#include "wcharUtil.h"
using namespace Win;

const int   UPDOWN_LOW = -10;
const int   UPDOWN_HIGH = 10;
const float SCALE = 1;


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
    buttonResetView.set(handle, IDC_BUTTON_VIEW_RESET);
    editViewX.set(handle, IDC_EDIT_VIEW_X);
    editViewY.set(handle, IDC_EDIT_VIEW_Y);
    editViewZ.set(handle, IDC_EDIT_VIEW_Z);
    editViewPitch.set(handle, IDC_EDIT_PITCH);
    editViewHeading.set(handle, IDC_EDIT_HEADING);
    editViewRoll.set(handle, IDC_EDIT_ROLL);
    textViewGL.set(handle, IDC_VIEW_GL);

    buttonResetModel.set(handle, IDC_BUTTON_MODEL_RESET);
    editModelX.set(handle, IDC_EDIT_POSITION_X);
    editModelY.set(handle, IDC_EDIT_POSITION_Y);
    editModelZ.set(handle, IDC_EDIT_POSITION_Z);
    editModelRotX.set(handle, IDC_EDIT_ROTATION_X);
    editModelRotY.set(handle, IDC_EDIT_ROTATION_Y);
    editModelRotZ.set(handle, IDC_EDIT_ROTATION_Z);
    textModelGL.set(handle, IDC_MODEL_GL);

    spinViewX.set(handle, IDC_SPIN_VIEW_X);
    spinViewX.setRange(UPDOWN_LOW, UPDOWN_HIGH);
    spinViewX.setPos(0);
    editViewX.setText(toWchar(spinViewX.getPos() * SCALE));

    spinViewY.set(handle, IDC_SPIN_VIEW_Y);
    spinViewY.setRange(UPDOWN_LOW, UPDOWN_HIGH);
    spinViewY.setPos(0);
    editViewY.setText(toWchar(spinViewY.getPos() * SCALE));

    spinViewZ.set(handle, IDC_SPIN_VIEW_Z);
    spinViewZ.setRange(UPDOWN_LOW, UPDOWN_HIGH);
    spinViewZ.setPos(10);
    editViewZ.setText(toWchar(spinViewZ.getPos() * SCALE));

    spinViewPitch.set(handle, IDC_SPIN_PITCH);
    spinViewPitch.setRange(-360, 360);
    spinViewPitch.setPos(0);
    editViewPitch.setText(toWchar(spinViewPitch.getPos()));

    spinViewHeading.set(handle, IDC_SPIN_HEADING);
    spinViewHeading.setRange(-360, 360);
    spinViewHeading.setPos(0);
    editViewHeading.setText(toWchar(spinViewHeading.getPos()));

    spinViewRoll.set(handle, IDC_SPIN_ROLL);
    spinViewRoll.setRange(-360, 360);
    spinViewRoll.setPos(0);
    editViewRoll.setText(toWchar(spinViewRoll.getPos()));

    spinModelX.set(handle, IDC_SPIN_POSITION_X);
    spinModelX.setRange(UPDOWN_LOW, UPDOWN_HIGH);
    spinModelX.setPos(0);
    editModelX.setText(toWchar(spinModelX.getPos() * SCALE));

    spinModelY.set(handle, IDC_SPIN_POSITION_Y);
    spinModelY.setRange(UPDOWN_LOW, UPDOWN_HIGH);
    spinModelY.setPos(0);
    editModelY.setText(toWchar(spinModelY.getPos() * SCALE));

    spinModelZ.set(handle, IDC_SPIN_POSITION_Z);
    spinModelZ.setRange(UPDOWN_LOW, UPDOWN_HIGH);
    spinModelZ.setPos(0);
    editModelZ.setText(toWchar(spinModelZ.getPos() * SCALE));

    spinModelRotX.set(handle, IDC_SPIN_ROTATION_X);
    spinModelRotX.setRange(-360, 360);
    spinModelRotX.setPos(0);
    editModelRotX.setText(toWchar(spinModelRotX.getPos()));

    spinModelRotY.set(handle, IDC_SPIN_ROTATION_Y);
    spinModelRotY.setRange(-360, 360);
    spinModelRotY.setPos(0);
    editModelRotY.setText(toWchar(spinModelRotY.getPos()));

    spinModelRotZ.set(handle, IDC_SPIN_ROTATION_Z);
    spinModelRotZ.setRange(-360, 360);
    spinModelRotZ.setPos(0);
    editModelRotZ.setText(toWchar(spinModelRotZ.getPos()));

    // elements for view matrix
    mv[0].set(handle, IDC_M_V_0);
    mv[1].set(handle, IDC_M_V_1);
    mv[2].set(handle, IDC_M_V_2);
    mv[3].set(handle, IDC_M_V_3);
    mv[4].set(handle, IDC_M_V_4);
    mv[5].set(handle, IDC_M_V_5);
    mv[6].set(handle, IDC_M_V_6);
    mv[7].set(handle, IDC_M_V_7);
    mv[8].set(handle, IDC_M_V_8);
    mv[9].set(handle, IDC_M_V_9);
    mv[10].set(handle, IDC_M_V_10);
    mv[11].set(handle, IDC_M_V_11);
    mv[12].set(handle, IDC_M_V_12);
    mv[13].set(handle, IDC_M_V_13);
    mv[14].set(handle, IDC_M_V_14);
    mv[15].set(handle, IDC_M_V_15);

    // elements for model matrix
    mm[0].set(handle, IDC_M_M_0);
    mm[1].set(handle, IDC_M_M_1);
    mm[2].set(handle, IDC_M_M_2);
    mm[3].set(handle, IDC_M_M_3);
    mm[4].set(handle, IDC_M_M_4);
    mm[5].set(handle, IDC_M_M_5);
    mm[6].set(handle, IDC_M_M_6);
    mm[7].set(handle, IDC_M_M_7);
    mm[8].set(handle, IDC_M_M_8);
    mm[9].set(handle, IDC_M_M_9);
    mm[10].set(handle, IDC_M_M_10);
    mm[11].set(handle, IDC_M_M_11);
    mm[12].set(handle, IDC_M_M_12);
    mm[13].set(handle, IDC_M_M_13);
    mm[14].set(handle, IDC_M_M_14);
    mm[15].set(handle, IDC_M_M_15);

    // elements for modelview matrix
    mmv[0].set(handle, IDC_M_MV_0);
    mmv[1].set(handle, IDC_M_MV_1);
    mmv[2].set(handle, IDC_M_MV_2);
    mmv[3].set(handle, IDC_M_MV_3);
    mmv[4].set(handle, IDC_M_MV_4);
    mmv[5].set(handle, IDC_M_MV_5);
    mmv[6].set(handle, IDC_M_MV_6);
    mmv[7].set(handle, IDC_M_MV_7);
    mmv[8].set(handle, IDC_M_MV_8);
    mmv[9].set(handle, IDC_M_MV_9);
    mmv[10].set(handle, IDC_M_MV_10);
    mmv[11].set(handle, IDC_M_MV_11);
    mmv[12].set(handle, IDC_M_MV_12);
    mmv[13].set(handle, IDC_M_MV_13);
    mmv[14].set(handle, IDC_M_MV_14);
    mmv[15].set(handle, IDC_M_MV_15);

    // textboxes for OpenGL function calls
    textViewGL.setFont(L"Courier New", 10);
    textModelGL.setFont(L"Courier New", 10);
}



///////////////////////////////////////////////////////////////////////////////
// change updown position
///////////////////////////////////////////////////////////////////////////////
int ViewFormGL::changeUpDownPosition(HWND handle, int position)
{
    if(handle == spinViewX.getHandle())
    {
        if(position >= UPDOWN_LOW && position <= UPDOWN_HIGH)
        {
            editViewX.setText(toWchar(position * SCALE));
            model->setCameraX(position * SCALE);
        }
    }
    else if(handle == spinViewY.getHandle())
    {
        if(position >= UPDOWN_LOW && position <= UPDOWN_HIGH)
        {
            editViewY.setText(toWchar(position * SCALE));
            model->setCameraY(position * SCALE);
        }
    }
    else if(handle == spinViewZ.getHandle())
    {
        if(position >= UPDOWN_LOW && position <= UPDOWN_HIGH)
        {
            editViewZ.setText(toWchar(position * SCALE));
            model->setCameraZ(position * SCALE);
        }
    }
    else if(handle == spinViewPitch.getHandle())
    {
        if(position >= -360 && position <= 360)
        {
            editViewPitch.setText(toWchar(position));
            model->setCameraAngleX((float)position);
        }
    }
    else if(handle == spinViewHeading.getHandle())
    {
        if(position >= -360 && position <= 360)
        {
            editViewHeading.setText(toWchar(position));
            model->setCameraAngleY((float)position);
        }
    }
    else if(handle == spinViewRoll.getHandle())
    {
        if(position >= -360 && position <= 360)
        {
            editViewRoll.setText(toWchar(position));
            model->setCameraAngleZ((float)position);
        }
    }
    else if(handle == spinModelX.getHandle())
    {
        if(position >= UPDOWN_LOW && position <= UPDOWN_HIGH)
        {
            editModelX.setText(toWchar(position * SCALE));
            model->setModelX(position * SCALE);
        }
    }
    else if(handle == spinModelY.getHandle())
    {
        if(position >= UPDOWN_LOW && position <= UPDOWN_HIGH)
        {
            editModelY.setText(toWchar(position * SCALE));
            model->setModelY(position * SCALE);
        }
    }
    else if(handle == spinModelZ.getHandle())
    {
        if(position >= UPDOWN_LOW && position <= UPDOWN_HIGH)
        {
            editModelZ.setText(toWchar(position * SCALE));
            model->setModelZ(position * SCALE);
        }
    }
    else if(handle == spinModelRotX.getHandle())
    {
        if(position >= -360 && position <= 360)
        {
            editModelRotX.setText(toWchar(position));
            model->setModelAngleX((float)position);
        }
    }
    else if(handle == spinModelRotY.getHandle())
    {
        if(position >= -360 && position <= 360)
        {
            editModelRotY.setText(toWchar(position));
            model->setModelAngleY((float)position);
        }
    }
    else if(handle == spinModelRotZ.getHandle())
    {
        if(position >= -360 && position <= 360)
        {
            editModelRotZ.setText(toWchar(position));
            model->setModelAngleZ((float)position);
        }
    }

    updateMatrices();
    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// set view matrix entries
///////////////////////////////////////////////////////////////////////////////
void ViewFormGL::setViewMatrix(float x, float y, float z, float p, float h, float r)
{
    spinViewX.setPos((int)(x / SCALE));
    editViewX.setText(toWchar(spinViewX.getPos()*SCALE));

    spinViewY.setPos((int)(y / SCALE));
    editViewY.setText(toWchar(spinViewY.getPos()*SCALE));

    spinViewZ.setPos((int)(z / SCALE));
    editViewZ.setText(toWchar(spinViewZ.getPos()*SCALE));

    spinViewPitch.setPos((int)p);
    editViewPitch.setText(toWchar(spinViewPitch.getPos()));

    spinViewHeading.setPos((int)h);
    editViewHeading.setText(toWchar(spinViewHeading.getPos()));

    spinViewRoll.setPos((int)r);
    editViewRoll.setText(toWchar(spinViewRoll.getPos()));

    updateMatrices();
}



///////////////////////////////////////////////////////////////////////////////
// set model matrix entries
///////////////////////////////////////////////////////////////////////////////
void ViewFormGL::setModelMatrix(float x, float y, float z, float rx, float ry, float rz)
{
    spinModelX.setPos((int)(x / SCALE));
    editModelX.setText(toWchar(spinModelX.getPos()*SCALE));

    spinModelY.setPos((int)(y / SCALE));
    editModelY.setText(toWchar(spinModelY.getPos()*SCALE));

    spinModelZ.setPos((int)(z / SCALE));
    editModelZ.setText(toWchar(spinModelZ.getPos()*SCALE));

    spinModelRotX.setPos((int)rx);
    editModelRotX.setText(toWchar(spinModelRotX.getPos()));

    spinModelRotY.setPos((int)ry);
    editModelRotY.setText(toWchar(spinModelRotY.getPos()));

    spinModelRotZ.setPos((int)rz);
    editModelRotZ.setText(toWchar(spinModelRotZ.getPos()));

    updateMatrices();
}



///////////////////////////////////////////////////////////////////////////////
// update all elements of 3 matrices and OpenGL function calls
///////////////////////////////////////////////////////////////////////////////
void ViewFormGL::updateMatrices()
{
    const float* matrix;
    std::wstringstream wss;
    int i;

    // convert number to string with limited decimal points
    wss << std::fixed << std::setprecision(2);

    matrix = model->getViewMatrixElements();
    for(i = 0; i < 16; ++i)
    {
        wss.str(L"");
        wss << matrix[i] << std::ends;
        mv[i].setText(wss.str().c_str());
    }

    matrix = model->getModelMatrixElements();
    for(i = 0; i < 16; ++i)
    {
        wss.str(L"");
        wss << matrix[i] << std::ends;
        mm[i].setText(wss.str().c_str());
    }

    matrix = model->getModelViewMatrixElements();
    for(i = 0; i < 16; ++i)
    {
        wss.str(L"");
        wss << matrix[i] << std::ends;
        mmv[i].setText(wss.str().c_str());
    }

    // update OpenGL function calls
    wss.str(L""); // clear
    wss << std::fixed << std::setprecision(0);
    wss << L"glRotatef(" << -model->getCameraAngleZ() << L", 0, 0, 1);\n"
        << L"glRotatef(" << -model->getCameraAngleY() << L", 0, 1, 0);\n"
        << L"glRotatef(" << -model->getCameraAngleX() << L", 1, 0, 0);\n"
        << L"glTranslatef(" << -model->getCameraX() << L", " << -model->getCameraY() << L", " << -model->getCameraZ() << L");\n"
        << std::ends;
    textViewGL.setText(wss.str().c_str());

    wss.str(L""); // clear
    wss << L"glTranslatef(" << model->getModelX() << L", " << model->getModelY() << L", " << model->getModelZ() << L");\n"
        << L"glRotatef(" << model->getModelAngleX() << L", 1, 0, 0);\n"
        << L"glRotatef(" << model->getModelAngleY() << L", 0, 1, 0);\n"
        << L"glRotatef(" << model->getModelAngleZ() << L", 0, 0, 1);\n"
        << std::ends;
    textModelGL.setText(wss.str().c_str());

	// unset floating format
    wss << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);
}

