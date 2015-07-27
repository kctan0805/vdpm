///////////////////////////////////////////////////////////////////////////////
// ViewFormGL.h
// ============
// View component of OpenGL dialog window
//
//  AUTHORL Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2008-09-15
// UPDATED: 2008-09-17
///////////////////////////////////////////////////////////////////////////////

#ifndef VIEW_FORM_GL_H
#define VIEW_FORM_GL_H

#include <windows.h>
#include "Controls.h"
#include "ModelGL.h"

namespace Win
{
    class ViewFormGL
    {
    public:
        ViewFormGL(ModelGL* model);
        ~ViewFormGL();

        void initControls(HWND handle);         // init all controls
        int  changeUpDownPosition(HWND handle, int position);
        void setViewMatrix(float x, float y, float z, float p, float h, float r);
        void setModelMatrix(float x, float y, float z, float rx, float ry, float rz);
        void updateMatrices();

    protected:

    private:
        ModelGL* model;
        HWND parentHandle;

        // controls for view section (camera)
        Win::Button    buttonResetView;
        Win::EditBox   editViewX;
        Win::EditBox   editViewY;
        Win::EditBox   editViewZ;
        Win::EditBox   editViewPitch;
        Win::EditBox   editViewHeading;
        Win::EditBox   editViewRoll;
        Win::UpDownBox spinViewX;
        Win::UpDownBox spinViewY;
        Win::UpDownBox spinViewZ;
        Win::UpDownBox spinViewPitch;
        Win::UpDownBox spinViewHeading;
        Win::UpDownBox spinViewRoll;
        Win::TextBox   textViewGL;

        // controls for model section
        Win::Button    buttonResetModel;
        Win::EditBox   editModelX;
        Win::EditBox   editModelY;
        Win::EditBox   editModelZ;
        Win::EditBox   editModelRotX;
        Win::EditBox   editModelRotY;
        Win::EditBox   editModelRotZ;
        Win::UpDownBox spinModelX;
        Win::UpDownBox spinModelY;
        Win::UpDownBox spinModelZ;
        Win::UpDownBox spinModelRotX;
        Win::UpDownBox spinModelRotY;
        Win::UpDownBox spinModelRotZ;
        Win::TextBox   textModelGL;

        // matrices
        Win::TextBox   mv[16];          // view matrix
        Win::TextBox   mm[16];          // model matrix
        Win::TextBox   mmv[16];         // modelview matrix
    };
}

#endif
