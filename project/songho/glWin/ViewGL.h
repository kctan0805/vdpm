///////////////////////////////////////////////////////////////////////////////
// ViewGL.h
// ========
// View component of OpenGL window
//
//  AUTHORL Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2006-07-10
// UPDATED: 2014-07-10
///////////////////////////////////////////////////////////////////////////////

#ifndef VIEW_GL_H
#define VIEW_GL_H

#include <windows.h>
#include "Controls.h"

namespace Win
{
    class ViewGL
    {
    public:
        ViewGL();
        ~ViewGL();

        bool createContext(HWND handle, int colorBits, int depthBits, int stencilBits);  // create OpenGL rendering context
        void closeContext(HWND handle);
        void swapBuffers();

        HDC getDC() const { return hdc; };
        HGLRC getRC() const { return hglrc; };

    protected:

    private:
        // member functions
        static bool setPixelFormat(HDC hdc, int colorBits, int depthBits, int stencilBits);
        static int findPixelFormat(HDC hdc, int colorbits, int depthBits, int stencilBits); // return best matched format ID

        HDC hdc;                                        // handle to device context
        HGLRC hglrc;                                    // handle to OpenGL rendering context
    };
}

#endif
