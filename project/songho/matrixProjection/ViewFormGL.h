///////////////////////////////////////////////////////////////////////////////
// ViewFormGL.h
// ============
// View component of OpenGL dialog window
//
//  AUTHORL Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2008-10-02
// UPDATED: 2008-10-07
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
        void setProjection(float l, float r, float b, float t, float n, float f);
        void updateProjectionMatrix();

    protected:

    private:
        ModelGL* model;
        HWND parentHandle;

        // controls
        Win::RadioButton radioOrthographic;
        Win::RadioButton radioPerspective;
        Win::RadioButton radioFill;
        Win::RadioButton radioLine;
        Win::RadioButton radioPoint;
        Win::Button      buttonReset;
        Win::EditBox     editLeft;
        Win::EditBox     editRight;
        Win::EditBox     editBottom;
        Win::EditBox     editTop;
        Win::EditBox     editNear;
        Win::EditBox     editFar;
        Win::UpDownBox   spinLeft;
        Win::UpDownBox   spinRight;
        Win::UpDownBox   spinBottom;
        Win::UpDownBox   spinTop;
        Win::UpDownBox   spinNear;
        Win::UpDownBox   spinFar;
        Win::TextBox     textGL;
        Win::TextBox     m[16];          // projection matrix

    };
}

#endif
