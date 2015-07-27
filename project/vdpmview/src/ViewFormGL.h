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
        void updateStatus();
        void reset();

        Win::CheckBox    checkColor;
        Win::CheckBox    checkTexture;
        Win::CheckBox    checkLighting;

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
        Win::EditBox     editTau;
        Win::EditBox     editAFaces;
        Win::EditBox     editGTime;
        Win::EditBox     editAmortize;
        Win::UpDownBox   spinTau;
        Win::UpDownBox   spinAFaces;
        Win::UpDownBox   spinGTime;
        Win::UpDownBox   spinAmortize;
        Win::TextBox     textStatus;
    };
}

#endif
